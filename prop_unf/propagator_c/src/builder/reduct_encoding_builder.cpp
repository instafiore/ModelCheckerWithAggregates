#include "reduct_encoding_builder.h"
#include "../propagator/non-hcf-propagator.h"


ReductEncodingBuilder* ReductEncodingBuilder::withConstraintAssumptions(NonHFCPropagator* prop){
    debug(INFO, "Creating assumptions for constraint");

    std::vector<std::string> body_cons ;
    int n = prop->component->internal_atoms.size() ;
    for(int i = 0; i < n; ++i){
        const auto& atom = prop->component->internal_atoms[i];
        std::string aux = createAux(std::to_string(atom)) ;
        body_cons.push_back(aux);
        std::vector<std::string> body_bin_clause  = {negateLiteral(aux), createLiteralString(atom)};
        withASPRule(prop, false, nullptr, &body_bin_clause);
        freshAtoms.emplace(aux);
    }

    withASPRule(prop, false, nullptr, &body_cons);
    return this;
}

std::string ReductEncodingBuilder::buildProgram(NonHFCPropagator* prop){
    debug(INFO, "Creating reduct program");

    std::string program = EncodingBuilder::buildProgram(prop);
    withConstraintAssumptions(prop);
    return program + __build__(prop);
}


EncodingBuilder* ReductEncodingBuilder::withRule(NonHFCPropagator* prop,Rule* rule){

    const clingo_atom_t* head = rule->head ;
    const clingo_literal_t* body = rule->body ;
    size_t clause_size = rule->head_size + rule->body_size ;
    int index_clause = 0 ;
    std::vector<std::string> body_vec ;

    for(int i = 0 ; i < rule->head_size; ++i){
        clingo_atom_t atom = head[i];
        std::string name = createLiteralString(atom);
        name = negateLiteral(name);
        body_vec.push_back(name);
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
    withASPRule(prop, false, nullptr, &body_vec);
    return this ;
}


EncodingBuilder* ReductEncodingBuilder::withWeightRule(NonHFCPropagator* prop, const WeightRule* rule){
    assert(rule->head_size == 1);
    clingo_atom_t aggr = rule->head[0];
    std::string aggr_str = createLiteralString(aggr);
    std::string aggr_str_neg = negateLiteral(aggr_str);
    clingo_weight_t b = rule->get_lower_bound();
    std::vector<AggregateElement> elementsRightDirection ;
    AggregateElement aggr_element = {.atomName = aggr_str_neg, .weight = b};
    elementsRightDirection.push_back(aggr_element);
    size_t n = rule->body_size ;
    
    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t l = rule->get_weighted_body()[i].literal;
        clingo_weight_t w = rule->get_weighted_body()[i].weight ;
        std::string l_str ;
        // ----
        if(l > 0){
            l_str = createLiteralString(l) ;
        }else{
            l_str = createLiteralString(std::abs(l));
            bool intern = prop->component->external_atoms.find(std::abs(l)) == prop->component->external_atoms.end() ;
            if(intern){
                l_str = negateLiteral(createPrime(l_str));
            }else{
                l_str = negateLiteral(l_str);
            }
        }
        // ----
        // AggregateElement e = {.atomName = createLiteralString(l), .weight = w};
        AggregateElement e = {.atomName = l_str, .weight = w};
        elementsRightDirection.push_back(e);
    }

    // Modelling one direction
    withASPSumRule(prop, false, nullptr,  elementsRightDirection, b);

    clingo_weight_t sum_w = 0 ; 
    for(int i = 0; i < n; ++i) sum_w += rule->get_weighted_body()[i].weight ; 

    std::vector<AggregateElement> elementsLeftDirection ;
    clingo_weight_t weight_left_dir = sum_w - b + 1 ; 
    aggr_element = {.atomName = aggr_str, .weight = weight_left_dir};
    elementsLeftDirection.push_back(aggr_element);
    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t l = rule->get_weighted_body()[i].literal;
        std::string l_str ;
        if(l > 0){
            l_str = negateLiteral(createLiteralString(l)) ;
        }else{
            l_str = createLiteralString(std::abs(l));
            bool intern = prop->component->external_atoms.find(std::abs(l)) == prop->component->external_atoms.end() ;
            if(intern){
                l_str = createPrime(l_str);
            }
        }
        clingo_weight_t w = rule->get_weighted_body()[i].weight ;
        AggregateElement e = {.atomName = l_str, .weight = w};
        elementsLeftDirection.push_back(e);
    }
    
    // Modelling the other direction
    withASPSumRule(prop, false, nullptr, elementsLeftDirection, weight_left_dir);

    return this ;
}
