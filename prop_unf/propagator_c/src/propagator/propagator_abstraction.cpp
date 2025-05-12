#include "propagator_abstraction.h"
#include <iostream>
#include <clingo.h>
#include "propagator_initializer.h"
#include <assert.h>

bool PropagatorAbstraction::init(clingo_propagate_init_t *_init){
    PropagatorInitializer::getInstance()->init(_init, *this);

    for (size_t i = 0; i < PropagatorInitializer::getInstance()->program.nt; i++){
        // TODO: parametrize
        debug(INFO, "Adding propagator for thread: ",i, " with method: ", this->params["method"], " and component: ", this->component->get_id());
        NonHFCPropagator* propagator = nullptr;
        
        if(this->params["method"] == "reduct"){ 
            propagator = new ReductBasedCheck(this->component, &PropagatorInitializer::getInstance()->program, this->params);
        }else if(this->params["method"] == "unfounded"){
            propagator = new UnfoundedBasedCheck(this->component, &PropagatorInitializer::getInstance()->program, this->params);
        }else if(this->params["method"] == "aspreduct"){
            propagator = new AspReductBasedCheck(this->component, &PropagatorInitializer::getInstance()->program, this->params);
        }else{
            debug(CRITICAL, "Error: method not found");
            exit(SETTINGS::ERROR_CODE);
        }
        this->propagators_imp.push_back(propagator);
    }

    std::vector<clingo_literal_t> to_watch_plit;
    for (size_t i = 0; i < PropagatorInitializer::getInstance()->program.nt; i++) to_watch_plit = this->propagators_imp[i]->init(); 
    
    size_t max_clause_size = this->propagators_imp[0]->component->getSize();
    // print("[size] max_clause_size: ",max_clause_size);
    clause_clingo = new clingo_literal_t[max_clause_size];
    
    
    for(clingo_literal_t atom: to_watch_plit){
        clingo_literal_t slit;
        if(prog->map_plit_slit->find(atom) == prog->map_plit_slit->end()){
            clingo_propagate_init_solver_literal(_init, atom, &slit);
            (*prog->map_plit_slit)[atom] = slit ;
        }
        slit = (*prog->map_plit_slit)[atom];
        update_map_value_vector(map_slit_plit_watched, slit, atom);       
        handle_error(clingo_propagate_init_add_watch(_init, slit));
    }

    size_t N = PropagatorInitializer::getInstance()->program.max_plit + 1;
    unfoundedToPropagate = new PerfectVector<clingo_atom_t>(N);
    return true;
}

bool PropagatorAbstraction::propagate(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size){
    auto start = start_timer();
    debug(INFO, "PropagatorAbstraction propagate");
    const clingo_assignment_t *assignment = clingo_propagate_control_assignment(control);
    size_t dl_prev = dl ;
    dl = clingo_assignment_decision_level(assignment);
    int td; 
    dl == 0 ? td = 0 : td = clingo_propagate_control_thread_id(control) ; 

    NonHFCPropagator* prop = propagators_imp[td];
    prop->controlProgram = control ;
    enablePropagation(prop);

    printPropagate(this, changes, size, control, dl, false);

    std::vector<clingo_literal_t> plits ;
    for (size_t i = 0; i < size; i++)
    {
        clingo_literal_t slit = changes[i];
        std::vector<clingo_literal_t> plit_list = map_slit_plit_watched[slit];
        extend_vector(plits, plit_list);
    }

    // Adding unfounded atoms to propagate
    bool conflict = propagateEnqueuedUnfounded(prop, control, dl_prev, false);
    if(conflict) disablePropagation(prop);
        
    
    const std::vector<clingo_atom_t>* X_plit = prop->onLiteralsTrue(plits, dl); // handled internally 
    if (X_plit != nullptr){ 
        propagateUnfoundedSet(control, *X_plit, dl, false);
    }

    auto elapsed = elapsed_time(start);
    addValue("propagate_time", elapsed.count());
    increaseCountStats();

    return true;
}

bool PropagatorAbstraction::propagateUnfoundedSet(void *control, const std::vector<clingo_atom_t>& X_plit, int dl, bool init=false){
    
    if(X_plit.empty()) return false ;

    int td ;
    init ? td = 0 : td = clingo_propagate_control_thread_id((clingo_propagate_control*) control); 
    
    NonHFCPropagator* prop = propagators_imp[td];

    size_t n = X_plit.size();
    for(int i = 0; i < n; ++i){
        clingo_atom_t unfounded = X_plit[i];
        clingo_atom_t slit_unfounded = (*prog->map_plit_slit)[unfounded] ;
        unfoundedToPropagate->push_back(slit_unfounded);
    }

    return propagateEnqueuedUnfounded(prop, control, dl, init); ;
}   

bool PropagatorAbstraction::propagateEnqueuedUnfounded(NonHFCPropagator* prop, void *control, int dl, bool init){
    while(!unfoundedToPropagate->empty()){
        clingo_atom_t unfounded = unfoundedToPropagate->back();
        unfoundedToPropagate->pop_back();
        if(propagateUnfounded(prop, unfounded, control, dl, false, true)){
            if(this->params["enqueue"] != SETTINGS::TRUE_STR){
                // not enqueuing 
                unfoundedToPropagate->clear();
            }
            return true ;
        }
    }
    return false ;
}

bool PropagatorAbstraction::propagateUnfounded(NonHFCPropagator* prop, clingo_atom_t atom, void *control, int dl, bool init, bool slit_b){
    assert(prop->getReason() != nullptr);
    const PerfectVector<clingo_literal_t>* R_plit = prop->getReason();
    size_t clause_size = R_plit->size() + 1;
    clingo_literal_t* clause = clause_clingo;
    clingo_literal_t slit = !slit_b ? (*prog->map_plit_slit)[atom] : atom;
    clause[0] = not_(slit); 

    assert(clause_size <= this->propagators_imp[0]->component->getSize());
    for (size_t i = 1; i < clause_size; i++) {
        clingo_literal_t r_plit =  (*R_plit)[i-1];
        clause[i] = (*prog->map_plit_slit)[r_plit];
    }

    return addClause(control, clause, clause_size, init);
}

bool PropagatorAbstraction::addClause(void *control, clingo_literal_t* clause, size_t clause_size, bool init){
    bool result_add_clause;
    init ? handle_error(clingo_propagate_init_add_clause((clingo_propagate_init*) control, clause, clause_size, &result_add_clause)) :
    handle_error(clingo_propagate_control_add_clause((clingo_propagate_control*) control, clause, clause_size, clingo_clause_type_learnt, &result_add_clause)) ;

    // debug(DEBUG,"Adding clause [slits]: ", arrayToString(clause, clause_size, ""));
    // propagation must return immediately, there is a conflict
    if (!result_add_clause){
        debug(DEBUG,"Conflict with clause [slits]: ", arrayToString(clause, clause_size, ""));
        // debug(DEBUG,"Conflict with clause [plits]: ", vectorToString(clause_plits, ""));
        return true ;
    }

    bool result_propagate;
    init ? handle_error(clingo_propagate_init_propagate((clingo_propagate_init*) control, &result_propagate)) :
    handle_error(clingo_propagate_control_propagate((clingo_propagate_control*)control, &result_propagate)) ;
    
    if (!result_propagate){ 
        debug(DEBUG,"Conflict with propagating [slits]: ", arrayToString(clause, clause_size, ""));
        // debug(DEBUG,"Conflict with propagating [plits]: ", clause_plit_str);
        return true ;
    }   

    return false ;
}

void PropagatorAbstraction::undo(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size){
    const clingo_assignment_t *assignment = clingo_propagate_control_assignment(control);
    int dl = clingo_assignment_decision_level(assignment);
    int td; 
    dl == 0 ? td = 0 : td = clingo_propagate_control_thread_id(control) ; 
    NonHFCPropagator* prop = propagators_imp[td];

    std::vector<clingo_literal_t> plit_list;
   
    
    for (size_t i = 0; i < size; i++)
    {
        clingo_literal_t slit = changes[i];
        extend_vector(plit_list, map_slit_plit_watched[slit]);
    }
    printUndo(this, changes, size, control, dl, td, false);
    prop->onLiteralsUndefined(plit_list);
}


std::string PropagatorAbstraction::compute_changes_str(const clingo_literal_t *changes, size_t size, int td){
    std::vector<std::string> changes_name_vec ; 
    for (size_t i = 0; i < size; i++)
    {
        clingo_literal_t slit = changes[i] ;
        
        for(clingo_literal_t atom: map_slit_plit_watched[slit]){
            assert(prog->atomNames != NULL);
            std::string name = getName(prog->atomNames, atom);
            std::string res = "[" + name + ", plit: " + std::to_string(atom) + ", slit: " +  std::to_string(slit) + "]";
            changes_name_vec.push_back(res);
        }
    }

    return vector_to_string(changes_name_vec,"");
}
