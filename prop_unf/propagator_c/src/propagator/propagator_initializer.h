#pragma once
#include <iostream>
#include <vector>
#include "../utils/utility.h"
#include "../utils/logger.h"
#include <clingo.h>
#include "../model/model.h"


class PropagatorAbstraction;

class PropagatorInitializer {

public:

    Program program;
    std::vector<clingo_literal_t>* lits;
    static PropagatorInitializer* instance;
    bool first = true ;
    

    PropagatorInitializer() {
        debug(INFO, "Initializer created");
    }

    static PropagatorInitializer* getInstance(){
        if(instance == nullptr) instance = new PropagatorInitializer() ;
        return instance ;
    }
    
    void init(clingo_propagate_init* _init, PropagatorAbstraction& propagator);

    ~PropagatorInitializer(){
        if(lits) delete lits ;
    }

    static void cleanup(){
        if(instance != nullptr) delete instance ;
    }


};


inline PropagatorInitializer* PropagatorInitializer::instance = nullptr;
