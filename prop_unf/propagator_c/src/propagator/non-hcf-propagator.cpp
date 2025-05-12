#include "non-hcf-propagator.h"
#include "../builder/encoding_builder.h"
#include <unordered_set>

const std::vector<clingo_literal_t>  NonHFCPropagator::init(){
    
    debug(INFO, "PropagatorImplementation init [component: ", component->get_id(),"]");

    int max_plit_local = 0 ;
    std::vector<clingo_literal_t> to_watch_plit;
    for(auto& internal_atom: component->internal_atoms){
        to_watch_plit.push_back(internal_atom);
        to_watch_plit.push_back(not_(internal_atom));
       if(max_plit_local < internal_atom) max_plit_local  = internal_atom;
    }
    for(auto& external_atom: component->external_atoms){
        to_watch_plit.push_back(external_atom);
        to_watch_plit.push_back(not_(external_atom));
       if(max_plit_local < external_atom) max_plit_local  = external_atom;
    }
    size_t N = prog->max_plit + 1 ;
    debug(INFO, " prog->max_plit for reason: ", N);
    reason = new PerfectVector<clingo_literal_t>(N);
    unfoundedSet = new PerfectVector<clingo_atom_t>(N);
    initOracle();
    debug(INFO,vectorToString(to_watch_plit, "to watch: "));
    return to_watch_plit;
}

void NonHFCPropagator::initOracle(){
    debug(INFO, "Starting oracle");
    
    const char *args[] = {"--seed=42"};
    handle_error(clingo_control_new(args, 1, NULL, NULL, 20, &controlOracle));
    clingo_part_t parts[] = {{"base", NULL, 0 }};

    // handle_error(clingo_control_register_observer(controlOracle, &observer, false, &programOracle));

    assert(builder != nullptr);
    std::string program = builder->buildProgram(this);
    debug(DEBUG,"Program input oracle [",component->get_id(),"]\n", program);
    // print(program);
    // exit(0);

    handle_error(clingo_control_add(controlOracle, "base", NULL, 0, program.c_str()));
    debug(INFO, "Starting grounding oracle");
    handle_error(clingo_control_ground(controlOracle, parts, 1, NULL, NULL));

    progOracle.init(controlOracle, nullptr, true);   
    for(auto [sym, plitOracle]: (*progOracle.atomNames)){
        std::string lit_str = from_symbol_to_string(sym);
        clingo_literal_t plitProgram = fromRep2plitProgram(lit_str);
        if(plitProgram == 0){
            debug(INFO, "plitProgram does not exists for ", lit_str);
            continue;
        }
        debug(INFO,"Mapping plits: ",lit_str, " plitProgram: ", plitProgram, " -> plitOracle: ",plitOracle);
        if(programAtom2oracleAtom.find(plitProgram) ==  programAtom2oracleAtom.end()){
            OracleAtom* op = createOracleAtom();
            programAtom2oracleAtom[plitProgram] = op ;
        }
        programAtom2oracleAtom[plitProgram]->addLiteral(lit_str, plitOracle);
    }
 
}

const std::vector<clingo_atom_t>* NonHFCPropagator::onLiteralsTrue(const std::vector<clingo_literal_t>& changes, const int& dl){
    clingo_literal_t lit = changes[0];
    updateDl(lit, dl);
    current_literal = lit ;
    InterpretationFunction* I = InterpretationFunction::getInstance();
    bool next_phase = updatePhase(changes, dl);
    assert(countDefined <= component->getSize());
    const std::vector<clingo_atom_t>* propagated = next_phase ? &propagate()->data() : nullptr;
    return propagated ;
}

bool NonHFCPropagator::isInternal(clingo_literal_t lit){
    clingo_atom_t a = std::abs(lit);
    bool external = this->component->external_atoms.find(a) != this->component->external_atoms.end();
    bool internal = !external;
    return internal;
}

bool NonHFCPropagator::isExternal(clingo_literal_t lit){
    return !isInternal(lit);
}

bool NonHFCPropagator::updatePhase(std::vector<clingo_literal_t> lits, int dl){
    InterpretationFunction* I = InterpretationFunction::getInstance();
    size_t countDefinedPrev = countDefined ;
    size_t countTruePrev = countTrue ;
    for(auto& l: lits){
        assert(I->get(l) != false);
        if(I->get(l) == true) continue;
        ++countDefined; 
        if(l > 0 && isInternal(l))  ++countTrue;
        I->set(l, true);
    }
   
    size_t n = component->getSize();
    size_t ni = component->internal_atoms.size();
    assert(countDefined <= n);
    high = countDefinedPrev + std::ceil(thresholdPartialCheck * n);
    low = countDefinedPrev  - std::ceil(thresholdPartialCheck * n);
    bool heuristic = (countDefined <= low || countDefined >= high) && countTrue >= ni * thresholdInternal;
    bool activatePartialCheck = params["partial_check"] == SETTINGS::TRUE_STR;
    bool partial_check = activatePartialCheck && heuristic;
    bool full_check = countDefined == n;
    return propagationEnabled && (partial_check || full_check);
}

const PerfectVector<clingo_atom_t>* NonHFCPropagator::propagate(){
    
    unfoundedSet->clear();
    reason->clear();
    debug(DEBUG, "Propagate phase started [",component->get_id(),"]");

    // Fixing interpretation
    fix();
    // std::unordered_set<clingo_atom_t> answersetOracle ;
    answersetOracle.clear();
    std::string oracleName = "oracle ["+std::to_string(component->get_id())+"]";
    debug(INFO,vectorToString(candidateModel, "Candidate Model: "));
    debug(DEBUG,vectorToStringName(prog->atomNames, candidateModel, "Candidate Model name: "));
    debug(DEBUG,vectorToStringName(progOracle.atomNames, assumptions, "Assumptions oracle Name: "));
    debug(INFO,vectorToString(assumptions, "Assumptions oracle: "));
    
    // Checker
    // assert(countDefined == component->getSize());
    auto start_checker = start_timer();
    int exitCode = solve(oracleName,controlOracle, assumptions, &answersetOracle, &progOracle);
    if(exitCode == SETTINGS::SAT_CODE){
        for(const auto& atom: candidateModel){
            OracleAtom* oracleAtom = programAtom2oracleAtom[atom];
            if(this->isUnfounded(oracleAtom, answersetOracle)){
                unfoundedSet->add(atom);
            }
        }
    }else{
        assert(exitCode == SETTINGS::UNSAT_CODE);
    }
    
    auto elasped_checker = elapsed_time(start_checker);    
    // addCheckerStats(elasped_checker.count());
    bool partial_check = countDefined < component->getSize();

    std::string checkType = partial_check ? "partial" : "full";
    std::string statTime = checkType + "_check_time";
    addValue(statTime, elasped_checker.count());
 
    debug(DEBUG,unorderedSetToStringName(progOracle.atomNames, answersetOracle, "Answerset oracle name: "));
    debug(INFO,unorderedSetToString(answersetOracle, "Answerset oracle: "));
    debug(DEBUG,vectorToStringName(prog->atomNames, unfoundedSet->data(), "Unfounded name set: "));

    debug(INFO,vectorToString(unfoundedSet->data(), "Unfounded set: "));
    debug(INFO,"Exit code oracle ",exitCode);
    

    if(unfoundedSet->size() > 0 && dl > 0 || partial_check) computeReasonOpt();

    // prinStructStats(false);
    prinStats(false);
    return unfoundedSet;
}

bool NonHFCPropagator::truthValue(clingo_literal_t l){
    // Assuming total interpretation
    InterpretationFunction* I = InterpretationFunction::getInstance();
    if(I->get(l) == SETTINGS::NONE){
        debug(ERROR, "Truth value is NONE for ", l, " it is not allowed!");
        setExitCode(SETTINGS::ERROR_CODE);
    }
    return I->get(l);
}


void NonHFCPropagator::computeReasonOpt(){
    auto start = start_timer();
    reason->clear();
    // if(unfoundedSet->size() == 0 || dl == 0) return ;
    debug(DEBUG, "Computing reason Opt");
    InterpretationFunction* I = InterpretationFunction::getInstance(); 
    for(const auto& r: component->rules){

        bool found = false ;
        clingo_literal_t reason_r = r->reasonHead(I, this, found);
        
        if(!found) continue;
        
        found = false ;
        clingo_literal_t reason_b = r->reasonBody(I, this, found);

        if(reason_b != 0){
            reason_r = reason_b ;
        }

        if(!found){
            if(reason_r == 0){
                debug(ERROR, "Error; ", r->toString(), " is not unfounded");
                assert(reason_r != 0);
            }
            reason->add(reason_r);
        }
    }

    for(const auto& r: component->aggregateRules){
        if(!r->isHeadUnfounded(*unfoundedSet)) continue; 
        r->computeReason(I, this, reason); 
    }

    printReason(prog->atomNames, reason, false);
    auto elasped = elapsed_time(start);
    // addReasonStats(elasped.count(), reason->size());

    addValue("reason_size", reason->size());
    addValue("reason_time", elasped.count());

}

const PerfectVector<clingo_literal_t>* NonHFCPropagator::getReason(){
    return reason;
}

void NonHFCPropagator::onLiteralsUndefined(const std::vector<clingo_literal_t> &plit_list){
    InterpretationFunction* I = InterpretationFunction::getInstance();
    countDefined -= plit_list.size(); 
    assert(countDefined >= 0);
    for(auto& plit: plit_list){
        if(plit < 0 && isInternal(plit)) --countTrue;
        I->set(plit, SETTINGS::NONE);
    } 
}

void NonHFCPropagator::updateDl(int lit, int new_dl) {
    if (new_dl != dl) {
        last_decision_lit = lit;  
    }
    dl = new_dl; 
}


void NonHFCPropagator::computeReason(){
    assert(false);
    reason->clear();
    debug(INFO, "Computing reason");
   

    InterpretationFunction* I = InterpretationFunction::getInstance(); 
    for(const auto& a: *unfoundedSet){
        for(const auto& r: component->rules){
            if(!r->isInHead(a)) continue; 
            if(r->isInBPlus(unfoundedSet->data())) continue; 
            
            clingo_literal_t lit = r->isFalseBody(I) ;
            bool falseBody = lit != 0 ;
            if(falseBody){
                reason->add(lit);
                continue;
            }
    
            clingo_atom_t h = r->isTrueHead(I, *unfoundedSet);
            bool trueHead = h != 0 ;
            if(trueHead){
                reason->add(not_(h));
                continue;
            }
            debug(CRITICAL, "Error in computing reason, atom: ", a, " is not unfounded !");
            setExitCode(SETTINGS::ERROR_CODE);
        }

        // aux :- #sum{...} >= b
        for(const auto& r: component->aggregateRules){
            if(!r->isInHead(a)) continue; 
            r->computeReason(I, this, reason); 
        }

    }
    printReason(prog->atomNames, reason, false);
}


