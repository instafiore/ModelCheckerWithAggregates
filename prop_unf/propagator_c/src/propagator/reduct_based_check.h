#pragma once
#include <clingo.h>
#include <unordered_map>
#include "../model/model.h"
#include "non-hcf-propagator.h"

class OracleAtomReduct: public OracleAtom{
public:
    clingo_literal_t prime = 0;
    clingo_literal_t aux = 0;
    void addLiteral(std::string rep, clingo_literal_t lit) override;
};

class ReductBasedCheck: public NonHFCPropagator{

public:

    ReductBasedCheck(Component* component,  Program* prog, std::unordered_map<std::string,std::string>& params);

    virtual ~ReductBasedCheck() override {
        debug(INFO, "ReductBasedCheck destroyed");
    }

    void fix() override;
    bool isUnfounded(OracleAtom* oracleAtom, std::unordered_set<clingo_atom_t>& answerset) override ;
    bool truthValue(clingo_literal_t l) override;
    OracleAtom* createOracleAtom(){ return new OracleAtomReduct();};
};

class AspReductBasedCheck: public ReductBasedCheck{

public:
    
    AspReductBasedCheck(Component* component,  Program* prog, std::unordered_map<std::string,std::string>& params);

    ~AspReductBasedCheck() override {
        debug(INFO, "ReductBasedCheck destroyed");
    }    

};