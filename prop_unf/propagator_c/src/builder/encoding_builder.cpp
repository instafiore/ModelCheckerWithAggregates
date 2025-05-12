
#include "encoding_builder.h"
#include "../propagator/non-hcf-propagator.h"
#include <assert.h>

std::string EncodingBuilder::__build__(NonHFCPropagator* prop){
    std::string res ;
    withFreshAtomChoices(prop);
    for(const auto& rule: program)  res += rule + "\n";
    program.clear();
    return res ;
}

std::string EncodingBuilder::buildProgram(NonHFCPropagator* prop){
    debug(INFO, "Creating program");

    for (const auto& rule : prop->component->rules) {
        withRule(prop, rule);
    }

    for (const auto& rule : prop->component->aggregateRules) {
        withWeightRule(prop, rule);
    }
    withChoicesForComponentAtoms(prop);
    return __build__(prop);
}

EncodingBuilder* EncodingBuilder::withFreshAtomChoices(NonHFCPropagator* prop){
    for(const auto& freshAtom: freshAtoms) withASPChoice(prop, {freshAtom});
    freshAtoms.clear();
    return this;
}

EncodingBuilder* EncodingBuilder::withASPRule(NonHFCPropagator* prop, bool choice, const std::vector<std::string>* head, const std::vector<std::string>* body){
    std::string str = createHead(choice, head) + createBody(body) + ".";
    program.push_back(str);
    return this;
}


EncodingBuilder* EncodingBuilder::withChoicesForComponentAtoms(NonHFCPropagator* prop){

    int n = prop->component->internal_atoms.size();
    for(auto& atom: prop->component->internal_atoms){
        withASPChoice(prop,{createLiteralString(atom)});
    }

    withChoicesForExternalAtoms(prop);

    return this ;
}

EncodingBuilder* EncodingBuilder::withChoicesForExternalAtoms(NonHFCPropagator* prop){

    for(auto& atom: prop->component->external_atoms){
        withASPChoice(prop,{createLiteralString(atom)});
    }

    return this ;
}



EncodingBuilder* EncodingBuilder::withASPChoice(NonHFCPropagator* prop, const std::vector<std::string>& literals){
    return withASPRule(prop, true, &literals, nullptr);
}

EncodingBuilder* EncodingBuilder::withASPConstraint(NonHFCPropagator* prop, const std::vector<std::string>& literals){
    return withASPRule(prop, false, nullptr, &literals);
}

EncodingBuilder* EncodingBuilder::withASPSumRule(NonHFCPropagator* prop,  bool choice, const std::vector<std::string>* head, const std::vector<AggregateElement> elements, int lb){
    std::string head_str = createHead(choice, head);
    if(head_str != "") head_str += " :- ";
    std::string rule = head_str + createSum(elements, lb, Operator::GE) + ".";
    program.push_back(rule);
    return this ;
}

EncodingBuilder* EncodingBuilder::withASPSumConstraint(NonHFCPropagator* prop, const std::vector<AggregateElement> elements, int lb){
    std::string constraint = ":- " + createSum(elements, lb, Operator::L) + ".";
    program.push_back(constraint);
    return this ;
}
