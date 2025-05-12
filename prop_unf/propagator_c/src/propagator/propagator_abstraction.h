#pragma once
#include <iostream>
#include "../utils/logger.h"
#include <string>
#include "../model/model.h" 
#include "non-hcf-propagator.h"
#include "reduct_based_check.h"
#include "unfounded_based_check.h"
#include <clingo.h>

// TODO:
// 1. Generalize the code to support multiple propagators, not just unfounded set propagator
// 1.1  PerfectVector<clingo_atom_t>* unfoundedToPropagate = nullptr; -> std:vector<clingo_literal_t*> clausesToPropagate;
// 1.2  bool propagateUnfoundedSet(void *control, const std::vector<clingo_atom_t>& unfoundedSet, int dl, bool init); -> propagateSet((void *control, const std::vector<clingo_literal_t>& set, int dl, bool init);
// 1.3  propagateUnfounded(NonHFCPropagator* prop, clingo_atom_t atom, void *control, int dl, bool init, bool slit=false); -> propagateLiteral(NonHFCPropagator* prop, clingo_literal_t literal, void *control, int dl, bool init, bool slit=false);
// 1.4  propagateEnqueuedUnfounded(NonHFCPropagator* prop, void *control, int dl, bool init); -> propagateEnqueuedClauses(NonHFCPropagator* prop, void *control, int dl, bool init);

class PropagatorAbstraction {

public:
    Component* component = nullptr;
    Program* prog;

    std::unordered_map<std::string, std::string>& params;
    std::unordered_map<clingo_literal_t, std::vector<clingo_literal_t>> map_slit_plit_watched ;

    clingo_literal_t* clause_clingo = nullptr;
    std::vector<NonHFCPropagator*> propagators_imp ; 
 
    PropagatorAbstraction(Component* component, std::unordered_map<std::string, std::string>& params): params(params), component(component) {
        debug(INFO, "PropagatorAbstraction created");
    }

    bool init(clingo_propagate_init_t *_init);
    bool propagate(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size);
    void undo(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size);
    bool addClause(void *control, clingo_literal_t* clause, size_t clause_size, bool init);
    
    // TODO: Generalize the code to support multiple propagators, not just unfounded set propagator
    bool propagateUnfoundedSet(void *control, const std::vector<clingo_atom_t>& unfoundedSet, int dl, bool init);
    bool propagateUnfounded(NonHFCPropagator* prop, clingo_atom_t atom, void *control, int dl, bool init, bool slit=false);
    bool propagateEnqueuedUnfounded(NonHFCPropagator* prop, void *control, int dl, bool init);
    
    void enablePropagation(NonHFCPropagator* prop){ prop->propagationEnabled = true; }
    void disablePropagation(NonHFCPropagator* prop){ prop->propagationEnabled = false; }
    PerfectVector<clingo_atom_t>* unfoundedToPropagate = nullptr;
    
    size_t dl = 0 ;
    std::string compute_changes_str(const clingo_literal_t *changes, size_t size, int td);
    ~PropagatorAbstraction() {
        // if (component) delete component;
        for(auto prop: propagators_imp){
            if(prop) delete prop;
        }
        if(clause_clingo != nullptr) delete[] clause_clingo;
        if(unfoundedToPropagate != nullptr) delete unfoundedToPropagate;
        debug(INFO, "PropagatorAbstraction destroyed");
    }
};