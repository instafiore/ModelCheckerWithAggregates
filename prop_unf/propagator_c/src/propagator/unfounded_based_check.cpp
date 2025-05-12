#include "unfounded_based_check.h"
#include "../builder/unfounded_encoding_builder.h"

UnfoundedBasedCheck::UnfoundedBasedCheck(Component* component,  Program* prog, std::unordered_map<std::string,std::string>& params): NonHFCPropagator(component, prog, params){
    debug(INFO, "PropagatorReduct created");
    builder = new UnfoundedEncodingBuilder();
}

void OracleAtomUnfoundedSet::addLiteral(std::string rep, clingo_literal_t lit){
    if(isULiteral(rep)){
        u = lit;
    }
    else if(!isHLiteral(rep)){
        atom = lit ;
    } 
}

bool UnfoundedBasedCheck::isUnfounded(OracleAtom* oracleAtom, std::unordered_set<clingo_atom_t>& answerset){
    OracleAtomUnfoundedSet* oracleAtomUnfoundedSet = (OracleAtomUnfoundedSet*) oracleAtom;
    return answerset.find(oracleAtomUnfoundedSet->u) != answerset.end();
}

bool UnfoundedBasedCheck::truthValue(clingo_literal_t l){
    InterpretationFunction* I = InterpretationFunction::getInstance(); 

    bool negative = l < 0 ;
    clingo_atom_t a = std::abs(l);
    bool external = this->component->external_atoms.find(a) != this->component->external_atoms.end();
    bool internal = !external;
    bool undefined = I->get(a) == SETTINGS::NONE;
    clingo_atom_t oracleAtom = this->programAtom2oracleAtom[a]->atom;
    bool trueInAnswersetOracle = this->answersetOracle.find(oracleAtom) != this->answersetOracle.end();
    bool truthValue;
    truthValue = I->get(a) == true || (undefined  && trueInAnswersetOracle);

    return negative ? !truthValue : truthValue;
}

void UnfoundedBasedCheck::fix(){
    int i = 0 ;
    
    InterpretationFunction* I = InterpretationFunction::getInstance();
    assumptions.clear();
    candidateModel.clear();
    for(const auto& atom: component->internal_atoms){
        OracleAtomUnfoundedSet* oracleAtom = (OracleAtomUnfoundedSet*)programAtom2oracleAtom[atom];
        // assert(I->get(atom) != SETTINGS::NONE);
        if(I->get(atom) == false){
            assumptions.push_back(not_(oracleAtom->atom));
            assert(oracleAtom->u != 0);
            assert(oracleAtom->atom != 0);
            assumptions.push_back(not_(oracleAtom->u));
        }
        else{
            assert(oracleAtom->atom != 0);
            if (I->get(atom) == true) assumptions.push_back(oracleAtom->atom);
            candidateModel.push_back(atom);
        };
    }

    for(auto& ext: component->external_atoms){
        clingo_atom_t atom = programAtom2oracleAtom[ext]->atom;
        if(I->get(ext) == SETTINGS::NONE) continue;
        // assert(I->get(ext) != SETTINGS::NONE);
        clingo_literal_t ass = I->get(ext) == true ? atom : not_(atom);
        assumptions.push_back(ass);
    }
}