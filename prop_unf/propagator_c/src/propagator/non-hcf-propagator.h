#pragma once
#include <clingo.h>
#include <unordered_map>
#include "../model/model.h"
#include "../utils/logger.h"
#include "../utils/utility.h"
#include <cmath>

class EncodingBuilder;

class OracleAtom{
public:
    clingo_literal_t atom = 0;
    virtual void addLiteral(std::string rep, clingo_literal_t lit) = 0;
    virtual ~OracleAtom() = default;
};

class NonHFCPropagator {
public:

    clingo_propagate_control_t *controlProgram;
    clingo_control_t *controlOracle = nullptr;
    EncodingBuilder* builder = nullptr ; 
    

    Program* prog ;
    Program progOracle ;
    Component* component = nullptr;
    PerfectVector<clingo_atom_t>* unfoundedSet = nullptr;
    PerfectVector<clingo_literal_t>* reason = nullptr  ;
    std::vector<clingo_atom_t> candidateModel ;
    bool propagationEnabled = true ;


    std::unordered_map<std::string, std::string> params;
    std::unordered_map<clingo_atom_t, OracleAtom*> programAtom2oracleAtom;

    size_t countDefined = 0 ;
    size_t countTrue = 0 ;
    std::vector<clingo_literal_t> assumptions;
    std::unordered_set<clingo_atom_t> answersetOracle ;
    float thresholdPartialCheck = 0.025 ;  // clingo-0.025 // default
    // float thresholdPartialCheck = 0.05 ;  // clingo-0.05
    // float thresholdPartialCheck = 0.10 ; // clingo-0.10
    // float thresholdInternal=0.25; // clingo-p-0.25 clingo 
    // float thresholdInternal=0.10; // clingo-p-0.10
    float thresholdInternal=0.20; // clingo-p-0.20 // default

    double low=0;
    size_t high=std::numeric_limits<size_t>::max();
    
    int last_decision_lit ;
    int dl = 0 ;
    clingo_literal_t current_literal;

    NonHFCPropagator(Component* component, Program* prog, std::unordered_map<std::string, std::string>& params) : params(params), prog(prog){
        this->component = component;
        debug(INFO, "PropagatorImplementation created");
        debug(DEBUG, "treshold_rate_partial_check: ", params["treshold_rate_partial_check"], " treshold_internal_partial_check: ", params["treshold_internal_partial_check"])
        thresholdPartialCheck = stof(params["treshold_rate_partial_check"]);
        thresholdInternal = stof(params["treshold_internal_partial_check"]);
    }

    virtual ~NonHFCPropagator() {
        if(builder) delete builder ;
        if(controlOracle) clingo_control_free(controlOracle);
        if(reason) delete reason ;
        if(unfoundedSet) delete unfoundedSet ;
        for(const auto& [key,value]: programAtom2oracleAtom){
            if(value != nullptr) delete value ; 
        }
        debug(INFO, "PropagatorImplementation destroyed");
    }

    const std::vector<clingo_literal_t>  init() ;
    const std::vector<clingo_atom_t>* onLiteralsTrue(const std::vector<clingo_literal_t>& changes, const int& dl);
    bool updatePhase(std::vector<clingo_literal_t> lits, int dl);
    const PerfectVector<clingo_atom_t>* propagate();
    const PerfectVector<clingo_literal_t>* getReason();
    void onLiteralsUndefined(const std::vector<clingo_literal_t> &plit_list);
    virtual bool truthValue(clingo_literal_t l);
    bool isInternal(clingo_literal_t lit);
    bool isExternal(clingo_literal_t lit);
    

    void updateDl(int lit, int new_dl);
    void computeReason();
    void computeReasonOpt();


    virtual OracleAtom* createOracleAtom() = 0 ;
    virtual bool isUnfounded(OracleAtom* oracleAtom, std::unordered_set<clingo_atom_t>& answerset) = 0 ;
    virtual void fix() = 0;

    void initOracle();
    clingo_ground_program_observer_t observer = {
        .rule = rule_callback,
        .weight_rule = weight_rule_callback,
    };
    
};