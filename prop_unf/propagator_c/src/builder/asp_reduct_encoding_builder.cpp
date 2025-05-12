#include "asp_reduct_encoding_builder.h"
#include "../propagator/non-hcf-propagator.h"



std::string AspReductEncodingBuilder::buildProgram(NonHFCPropagator* prop){
    debug(INFO, "Creating asp reduct program");

    for (const auto& rule : prop->component->rules) {
        withRule(prop, rule);
    }

    for (const auto& rule : prop->component->aggregateRules) {
        withWeightRule(prop, rule);
    }
    withChoicesForExternalAtoms(prop);
    withConstraintAssumptions(prop);
    return __build__(prop);
}

EncodingBuilder* AspReductEncodingBuilder::withRule(NonHFCPropagator* prop,Rule* rule){

    const clingo_atom_t* head = rule->head ;
    const clingo_literal_t* body = rule->body ;
    size_t clause_size = rule->head_size + rule->body_size ;
    int index_clause = 0 ;
    std::vector<std::string> head_vec ;
    std::vector<std::string> body_vec ;

    for(int i = 0 ; i < rule->head_size; ++i){
        clingo_atom_t atom = head[i];
        std::string name = createLiteralString(atom);
        head_vec.push_back(name);
        ++index_clause;
    }

    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t literal = body[i];
        bool negative = literal < 0 ;
        bool ext = prop->component->external_atoms.find(std::abs(literal)) != prop->component->external_atoms.end() ;
        bool prime = negative && !ext;
        literal = std::abs(literal);
        std::string name = createLiteralString(literal);
        if(prime){
            name = createPrime(name) ;
            freshAtoms.emplace(name);
            name = negateLiteral(name) ; 
        }else if (negative) name = negateLiteral(name) ;
        body_vec.push_back(name);
        ++index_clause;
    }
    withASPRule(prop, rule->isChoice(), &head_vec, &body_vec);
    // debug(INFO,"From: ", rule->toString(), " to: ", constraint);
    return this ;
}


EncodingBuilder* AspReductEncodingBuilder::withWeightRule(NonHFCPropagator* prop, const WeightRule* rule){
    assert(rule->head_size == 1);
    clingo_atom_t aggr = rule->head[0];
    std::string aggr_str = createLiteralString(aggr);
    std::vector<std::string> head_vec = {aggr_str};
    clingo_weight_t b = rule->get_lower_bound();
    std::vector<AggregateElement> elements ;
    size_t n = rule->body_size ;
    
    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t l = rule->get_weighted_body()[i].literal;
        clingo_weight_t w = rule->get_weighted_body()[i].weight ;
        AggregateElement e = {.atomName = createLiteralString(l), .weight = w};
        elements.push_back(e);
    }

    withASPSumRule(prop, rule->isChoice(), &head_vec, elements, b);
    return this ;
}
