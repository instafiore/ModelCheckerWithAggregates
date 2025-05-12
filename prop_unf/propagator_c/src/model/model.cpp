#include "model.h"
#include <thread>
#include <chrono>
#include "../propagator/non-hcf-propagator.h"

std::string Program::toString () const {
    std::string str = "";
    for(auto& rule: allRules){
        str += rule->toString() + "\n";
    }
    return str;
}

void Program::add_rule(Rule* rule) { 

    allRules.push_back(rule);
    RULE_TYPE type ;

    if(rule->head_size == 0){
        type = CONS;
        return ;
    }

    if(rule->isChoice()){
        type = CHOICE;
    }else if (WeightRule* aggregate = dynamic_cast<WeightRule*>(rule)){
        type = AGGR;
    }else if(rule->head_size > 1){
        type = DISJ;
    }else{
        type = NR;
    }

    rule->setType(type);

    for(int i = 0; i < rule->head_size; ++i){
        clingo_literal_t plit = rule->head[i];
        if(plit > max_plit) max_plit = plit; 
    }

    for(int i = 0; i < rule->body_size; ++i){
        clingo_literal_t plit ;
        if(type == AGGR) plit = dynamic_cast<WeightRule*>(rule)->get_weighted_body()[i].literal;
        else plit = rule->body[i];
        if(plit > max_plit) max_plit = plit; 
    }

    if(rule->body_size > 0)
        for (size_t i = 0; i < rule->head_size; i++){
            clingo_atom_t atom = rule->head[i];
            for (size_t j = 0; j < rule->body_size; j++){
                WeightRule* aggregate;
                if(type == AGGR) aggregate = dynamic_cast<WeightRule*>(rule);
                clingo_literal_t literal ;
                if(type == AGGR){
                    literal = aggregate->get_weighted_body()[j].literal;
                }else{
                    literal = rule->body[j];
                }
                // debug(INFO,"Adding edge: ", literal, " -> ", atom);
                if(literal > 0) dependency_graph.addEdge(literal, atom);
            }
        }
}

std::string Rule::toString() const{
    std::string str = "";
    if (choice) str += "{";
    for(size_t i = 0; i < head_size; i++){
        assert(head[i] != NULL);
        std::string del = choice ? ";" : "|";
        str += std::to_string(head[i]) + (i < head_size - 1 && head_size > 1 ? del : "");    
    }
    if (choice) str += "}";
    if(body_size > 0) str +=":- ";
    for(size_t i = 0; i < body_size; i++){
        str += std::to_string(body[i]) + (i < body_size - 1 ? ", " : "");
    }
    return str + "." ;
}

bool Rule::isInHead(clingo_atom_t atom){
    for(int i = 0; i < head_size; ++i)
        if(head[i] == atom) return true ;
    return false ;
}
bool Rule::isInBPlus(clingo_atom_t atom){
    for(int i = 0; i < body_size; ++i)
        if(body[i] == atom) return true ;
    return false ;
}

bool Rule::isInBPlus(std::vector<clingo_atom_t> atoms){
    
    for(const auto& atom: atoms)
        for(int i = 0; i < body_size; ++i)
            if(body[i] == atom) return true ;
                return false ;
}

bool Rule::isInBMinus(clingo_atom_t atom){
    for(int i = 0; i < body_size; ++i)
        if(body[i] == not_(atom)) return true ;
    return false ;
}

bool Rule::isTrueBody(const InterpretationFunction* I){
    for(int i = 0; i < body_size; ++i){
        clingo_literal_t lit = body[i] ;
        if(I->get(lit) == false) return false ;
    }
    return true ;
}

clingo_literal_t Rule::isFalseBody(const InterpretationFunction* I){
    for(int i = 0; i < body_size; ++i){
        clingo_literal_t lit = body[i] ;
        if(I->get(lit) == false) return lit ;
    }
    return 0 ;
}

clingo_atom_t Rule::isTrueHead(const InterpretationFunction* I, const PerfectVector<clingo_atom_t>& X){
    for(int i = 0; i < head_size; ++i){
        clingo_atom_t a = head[i] ;
        if(I->get(a) == true && !X.has(a)) return a ;
    }
    return 0 ;
}

clingo_literal_t Rule::reasonHead(const InterpretationFunction* I, NonHFCPropagator* prop, bool &found){
    found = false ;
    const PerfectVector<clingo_atom_t> & X = *prop->unfoundedSet;
    clingo_atom_t h = 0 ;
    clingo_atom_t h_undef = 0 ;
    for(int i = 0; i < head_size; ++i){
        clingo_atom_t a = head[i] ;
        bool truthValue = prop->truthValue(a);
        if(X.has(a)){ 
            found = true ;
            if(h != 0 ) break;
        }
        else if(truthValue){ 
            if(I->get(a) == SETTINGS::NONE) h_undef = a ;
            else h = a ;
            if(found) break ;
        }
    }
    if(h==0) h=h_undef;
    return not_(h);
}


clingo_literal_t Rule::reasonBody(const InterpretationFunction* I, NonHFCPropagator* prop, bool &found){
    found = false ;
    clingo_literal_t reason_r = 0 ;
    clingo_literal_t reason_r_undef = 0 ;
    const PerfectVector<clingo_atom_t> & X = *prop->unfoundedSet;
    for(int i = 0; i < body_size; ++i){
        clingo_literal_t b = body[i] ;
        bool truthValue = prop->truthValue(b);
        if(!truthValue){
            if(I->get(b) == SETTINGS::NONE) reason_r_undef = b ;
            else reason_r = b ;
            continue;
        }
        if(X.has(b)){ 
            found = true ;
            return 0;
        }
    }
    if(reason_r == 0) reason_r = reason_r_undef;
    return reason_r ;
}

bool WeightRule::isHeadUnfounded(const PerfectVector<clingo_atom_t>& unfoundedSet){

    for(int i = 0; i < head_size; ++i){
        clingo_atom_t b = head[i] ;
        if(unfoundedSet.has(b)) return true ;
    }
    return false ;
}

int WeightRule::getSum(InterpretationFunction* I) const{
    int sum = 0 ; 
    for(int i = 0; i < body_size; ++i){
        clingo_weighted_literal_t lit  = weighted_body[i];
        sum += lit.weight * (I->get(lit.literal) == true) ; 
    }
    return sum ;
}


void WeightRule::computeReason(InterpretationFunction* I, NonHFCPropagator* prop, PerfectVector<clingo_literal_t>* reason) const{
    int sum = getSum(I);
    for(int i = 0; i < body_size; ++i){
        clingo_weighted_literal_t lit  = weighted_body[i];
        if(I->get(lit.literal) == true) continue; 
        // assert(I->get(lit.literal) == false);
        bool truthValue = prop->truthValue(lit.literal);
        // assert(I->get(lit.literal) == false);
        if(truthValue) continue ;
        clingo_weight_t w = lit.weight;
        if(sum + w >= lower_bound){
            reason->add(lit.literal);
        }else{
            sum += w;
        }
    }
}

std::string WeightRule::toString() const{
    std::string str = "";
    if (choice) str += "{ ";
    for(size_t i = 0; i < head_size; i++){
        if(i > 0 && !choice) str +="| ";
        str += std::to_string(head[i]) + (i < head_size - 1 ? " " : "");    
    }
    if (choice) str += "}";
    if(body_size > 0) str +=":- ";
    str += "#sum{ ";
    for(size_t i = 0; i < body_size; i++){
        str += std::to_string(weighted_body[i].weight) + ": " + std::to_string(weighted_body[i].literal) + (i < body_size - 1 ? "; " : "");
    }
    str += "} >= " + std::to_string(lower_bound);
    return str +  ".";
}

void Component::add_rule(Rule* rule){ 
    // print("Adding rule: ", rule->toString());
    allRules.push_back(rule);
    if(rule->getType() == AGGR) aggregateRules.push_back(dynamic_cast<WeightRule*>(rule)) ;
    else if(rule->getType() == CONS)   constraints.push_back(rule);
    else if(rule->getType() == NR || true)   rules.push_back(rule);
    // else if(rule->getType() == CHOICE) choice_rules.push_back(rule);
    // else if(rule->getType() == DISJ)   disjunctions.push_back(rule);
    else assert(false);
}

void Program::computeNonHFCComponents(){
    debug(INFO,"computeNonHFCComponents");

    dependency_graph.computeStrongConnectedComponents();

    // print("[size] max_plit: ",max_plit);
    map_atom_component = new PerfectHash<clingo_atom_t>(max_plit+1,SETTINGS::NONE);
    size_t nc = 0 ;
    double size = 0.0 ;
    for(unsigned i = 0; i < dependency_graph.numberComponents() ; i++){
        bool isNonHFC = false;
        if(dependency_graph.componentSize(i) == 1){
            continue; 
        }
        Component* component = new Component(nc);
        component->internal_atoms.swap(dependency_graph.getComponent(i));
        for(const auto& a: component->internal_atoms){
            map_atom_component->set(a,nc); 
        }
        components.push_back(component);
        ++nc ;
    }
    std::vector<bool> isNonHCF(nc,false);
    for(unsigned i = 0; i < nc; i++){
        for(auto& rule: allRules){
            size_t count_head_atom = 0 ;
            std::vector<clingo_atom_t> external_tmp ;
            for(size_t j = 0 ; j < rule->head_size; ++j){
                clingo_atom_t head = rule->head[j];
                if(map_atom_component->get(head) == i) ++count_head_atom;
                else external_tmp.push_back(head);
            }
            if(count_head_atom >= 1){
                for(const auto& e: external_tmp){
                    components[i]->external_atoms.emplace(e);
                }

                for(size_t j = 0; j < rule->body_size; j++){
                    clingo_literal_t literal;
                    if(WeightRule* aggregate = dynamic_cast<WeightRule*>(rule)){
                        literal = aggregate->get_weighted_body()[j].literal;
                    }else{
                        literal = rule->body[j];
                    }
                    clingo_atom_t body_atom = std::abs(literal);
                    if(map_atom_component->get(body_atom) != i) components[i]->external_atoms.emplace(body_atom);
                    // print("map_atom_component->get(body_atom): ",map_atom_component->get(body_atom));
                }
                if(std::find(components[i]->allRules.begin(), components[i]->allRules.end(), rule) == components[i]->allRules.end()){
                    components[i]->add_rule(rule);
                }
            }
            if(count_head_atom >= 2) isNonHCF[i] = true ;
            // print(rule->toString(), " component: ",i, " count_head_atom: ",count_head_atom);
        }
    }

    // print(vector_to_string(isNonHCF, "isNonHCF: "));
    int i = 0 ; 
    while(i < nc){
        if(!isNonHCF[i]){
            Component* c = components[i] ;
            swap(components[i], components[nc-1]) ;
            swap(isNonHCF[i], isNonHCF[nc-1]) ;
            delete c ;
            components.pop_back();
            --nc ;
        }else{
            ++i ;
        }
    }
}

void Program::init(clingo_control* ctl, clingo_propagate_init* ctl_init, bool print_b = true){
    if(ctl != nullptr) this->ctl = ctl ;
    this->nt = ctl_init != nullptr ? clingo_propagate_init_number_of_threads(ctl_init) : 1 ;
    
    if(print_b) debug(INFO, "[init] number of threads ", this->nt);
    clingo_symbolic_atoms_t const *symbolic_atoms;
    clingo_symbolic_atom_iterator_t symbolic_atoms_it, symbolic_atoms_ie;
    
    handle_error(ctl_init != nullptr ? clingo_propagate_init_symbolic_atoms(ctl_init, &symbolic_atoms) : clingo_control_symbolic_atoms(ctl, &symbolic_atoms));
    handle_error(clingo_symbolic_atoms_end(symbolic_atoms, &symbolic_atoms_ie));

    handle_error(clingo_symbolic_atoms_begin(symbolic_atoms, NULL, &symbolic_atoms_it));
    while (true) {
        
        bool equal;
        clingo_literal_t plit;
        clingo_literal_t slit;
        clingo_symbol_t symbol;

        // stop iteration if the end is reached
        handle_error(clingo_symbolic_atoms_iterator_is_equal_to(symbolic_atoms, symbolic_atoms_it, symbolic_atoms_ie, &equal));
        
        if (equal) { break; }
        
        handle_error(clingo_symbolic_atoms_symbol(symbolic_atoms, symbolic_atoms_it, &symbol));
        std::string symbol_str = from_symbol_to_string(symbol) ;

        handle_error(clingo_symbolic_atoms_literal(symbolic_atoms, symbolic_atoms_it, &plit));
        handle_error(ctl_init != nullptr ? clingo_propagate_init_solver_literal(ctl_init, plit, &slit) : true);

        if (plit > this->max_plit) { this->max_plit = plit; }

        if(print_b) debug(INFO,"[init] symbol: ", symbol_str, " symbol: ", symbol, " plit: ", plit, " slit: ", slit);

        this->atomNames->emplace(symbol, plit);
        // this->plit2rep->emplace(plit, symbol_str);
        // this->plit2rep->emplace(not_(plit), "not "+ symbol_str);
        this->rep2plit->emplace(symbol_str, plit);
        // this->rep2plit->emplace("not "+symbol_str, not_(plit));
        if(ctl_init != nullptr){
            (*this->map_plit_slit)[plit] = slit ; 
            (*this->map_plit_slit)[not_(plit)] = not_(slit) ; 
            update_map_value_vector<clingo_literal_t, clingo_literal_t>(*this->map_slit_plit, slit, plit);
            update_map_value_vector<clingo_literal_t, clingo_literal_t>(*this->map_slit_plit, not_(slit), not_(plit));
        }
    
        clingo_symbolic_atoms_next(symbolic_atoms, symbolic_atoms_it, &symbolic_atoms_it);
    }
   
}

std::string Component::toString() const{
    std::string str = "Component " + std::to_string(id) + "\nrules: \n";
    for(auto& rule: allRules){
        str += rule->toString() + "\n";
    }
    str += "internal atoms: ";
    for(auto& atom: internal_atoms){
        str += std::to_string(atom) + " ";
    }
    str += "\nexternal atoms: ";
    for(auto& atom: external_atoms){
        str += std::to_string(atom) + " ";
    }
    return str;
}
