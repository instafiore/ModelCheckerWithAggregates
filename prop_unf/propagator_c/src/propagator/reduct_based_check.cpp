#include "reduct_based_check.h"
#include "../builder/reduct_encoding_builder.h"
#include "../builder/asp_reduct_encoding_builder.h"
#include "../settings.h"

void OracleAtomReduct::addLiteral(std::string rep, clingo_literal_t lit){
    if(isPrimeOracleLiteral(rep)){
        prime = lit;
    }
    else if(isAuxOracleLiteral(rep)){
        aux = lit;
    }
    else atom = lit ;
}

ReductBasedCheck::ReductBasedCheck(Component* component,  Program* prog, std::unordered_map<std::string,std::string>& params): NonHFCPropagator(component, prog, params){
    debug(INFO, "ReductBasedCheck created");
    builder = new ReductEncodingBuilder();
}

AspReductBasedCheck::AspReductBasedCheck(Component* component,  Program* prog, std::unordered_map<std::string,std::string>& params): ReductBasedCheck(component, prog, params){
    debug(INFO, "AspReductBasedCheck created");
    builder = new AspReductEncodingBuilder();
}

bool ReductBasedCheck::isUnfounded(OracleAtom* oracleAtom, std::unordered_set<clingo_atom_t>& answerset){
    return answerset.find(oracleAtom->atom) == answerset.end();
}

bool ReductBasedCheck::truthValue(clingo_literal_t l){
    InterpretationFunction* I = InterpretationFunction::getInstance(); 
    // return I->get(l) == true;
    bool negative = l < 0 ;
    clingo_atom_t a = std::abs(l);
    bool external = this->component->external_atoms.find(a) != this->component->external_atoms.end();
    bool internal = !external;
    bool undefined = I->get(a) == SETTINGS::NONE;
    clingo_atom_t oracleAtom = this->programAtom2oracleAtom[a]->atom;
    bool trueInAnswersetOracle = this->answersetOracle.find(oracleAtom) != this->answersetOracle.end();
    bool truthValue;
    if(external){
        truthValue = I->get(a) == true || (undefined  && trueInAnswersetOracle);
    }else{
        truthValue = undefined ? !negative : I->get(a) == true ; 
    }

    return negative ? !truthValue : truthValue;
}


void ReductBasedCheck::fix(){
    int i = 0 ;
    
    InterpretationFunction* I = InterpretationFunction::getInstance();
    assumptions.clear();
    candidateModel.clear();
    for(const auto& atom: component->internal_atoms){
        // assert(I->get(atom) != SETTINGS::NONE);
        OracleAtomReduct* oracleAtom = (OracleAtomReduct*)programAtom2oracleAtom[atom];
        if(I->get(atom) == SETTINGS::NONE) {
            assumptions.push_back(oracleAtom->atom);
            if(oracleAtom->prime != 0) assumptions.push_back(not_(oracleAtom->prime));
            if(oracleAtom->aux != 0) assumptions.push_back(oracleAtom->aux);
            continue;
        }
        
        if(I->get(atom) == false) assumptions.push_back(not_(oracleAtom->atom));
        else candidateModel.push_back(atom);
        if(oracleAtom->prime != 0) I->get(atom) == true  ? assumptions.push_back(oracleAtom->prime) : assumptions.push_back(not_(oracleAtom->prime));
        if(oracleAtom->aux != 0 && I->get(atom) == false ) assumptions.push_back(oracleAtom->aux);
    }

    for(auto& ext: component->external_atoms){
        // assert(I->get(ext) != SETTINGS::NONE);
        if(I->get(ext) == SETTINGS::NONE) continue;
        clingo_atom_t atom = programAtom2oracleAtom[ext]->atom;
        clingo_literal_t ass = I->get(ext) == true ? atom : not_(atom);
        assumptions.push_back(ass);
    }

}

