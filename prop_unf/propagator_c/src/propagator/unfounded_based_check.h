#pragma once
#include "non-hcf-propagator.h"

class OracleAtomUnfoundedSet: public OracleAtom{
public:
    clingo_literal_t u = 0;
    void addLiteral(std::string rep, clingo_literal_t lit) override;
};


class UnfoundedBasedCheck : public NonHFCPropagator{

public:
 
    UnfoundedBasedCheck(Component* component, Program* prog, std::unordered_map<std::string,std::string>& params);

    ~UnfoundedBasedCheck() {
        debug(INFO, "UnfoundedBasedCheck destroyed");
    }
    OracleAtom* createOracleAtom(){ return new OracleAtomUnfoundedSet();}
    bool isUnfounded(OracleAtom* oracleAtom, std::unordered_set<clingo_atom_t>& answerset) override;
    bool truthValue(clingo_literal_t l) override;
    void fix() override;
};