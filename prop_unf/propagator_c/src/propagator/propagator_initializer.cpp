#include "propagator_initializer.h"
#include "propagator_abstraction.h"

void PropagatorInitializer::init(clingo_propagate_init* _init, PropagatorAbstraction& propagator) {
    
    if(!first){
        propagator.prog = &program;
        return ;
    }
    first = false ;

    // Program prog ;
    program.init(nullptr, _init, true);
    
    // print("[size] program.max_plit: ", program.max_plit);
    lits = new std::vector<clingo_literal_t>{program.max_plit};

    // initializing interpretation function
    

    std::vector<clingo_literal_t> facts = get_map_value_vector<clingo_literal_t, clingo_literal_t>((*program.map_slit_plit), 1);
    extend_vector(*lits, facts);

    propagator.prog = &program;
}
