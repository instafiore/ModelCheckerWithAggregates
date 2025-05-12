#pragma once
#include <iostream>
#include <clingo.h>
#include "../model/model.h"

class NonHFCPropagator ;

class EncodingBuilder{

private:
    std::vector<std::string> program ;

protected:
    std::string __build__(NonHFCPropagator* prop);
    std::unordered_set<std::string> freshAtoms ; 

public:

    virtual ~EncodingBuilder(){}
    virtual std::string buildProgram(NonHFCPropagator* prop);

    EncodingBuilder* withASPRule(NonHFCPropagator* prop, bool choice, const std::vector<std::string>* head, const std::vector<std::string>* body);
    EncodingBuilder* withASPSumRule(NonHFCPropagator* prop,  bool choice, const std::vector<std::string>* head, const std::vector<AggregateElement> elements, int lb);
    EncodingBuilder* withASPSumConstraint(NonHFCPropagator* prop, const std::vector<AggregateElement> elements, int lb);
    EncodingBuilder* withASPChoice(NonHFCPropagator* prop,  const std::vector<std::string>& literals);
    EncodingBuilder* withASPConstraint(NonHFCPropagator* prop, const std::vector<std::string>& literals);
    EncodingBuilder* withChoicesForComponentAtoms(NonHFCPropagator* prop) ;
    EncodingBuilder* withChoicesForExternalAtoms(NonHFCPropagator* prop) ;
    EncodingBuilder* withFreshAtomChoices(NonHFCPropagator* prop);
    
    virtual EncodingBuilder* withRule(NonHFCPropagator* prop,Rule* rule) = 0;
    virtual EncodingBuilder* withWeightRule(NonHFCPropagator* prop, const WeightRule* rule) = 0 ;

};