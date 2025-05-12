#include "unfounded_encoding_builder.h"
#include "../propagator/non-hcf-propagator.h"


std::string UnfoundedEncodingBuilder::buildProgram(NonHFCPropagator* prop){
    debug(INFO, "Creating reduct program");
    std::string program = EncodingBuilder::buildProgram(prop);
    withHandC(prop);
    
    return program + __build__(prop);
}

UnfoundedEncodingBuilder* UnfoundedEncodingBuilder::withHandC(NonHFCPropagator* prop) {
    std::vector<std::string> c ;
    for(auto& atom: prop->component->internal_atoms){
        std::string p = createLiteralString(atom);
        std::string u = createUAtom(p);
        std::string h = createHAtom(p);

        freshAtoms.emplace(h);
        freshAtoms.emplace(u);
 
        // one direction   
        std::vector<std::string> oneDirection1 = {h, negateLiteral(p)};
        std::vector<std::string> oneDirection2 = {h, u};
        withASPConstraint(prop, oneDirection1);
        withASPConstraint(prop, oneDirection2);

        // other direction
        std::vector<std::string> otherDirection = {negateLiteral(h), p, negateLiteral(u)};
        withASPConstraint(prop, otherDirection);

        c.push_back(negateLiteral(u));
    }

    withASPConstraint(prop, c);
    return this ;
}


EncodingBuilder* UnfoundedEncodingBuilder::withRule(NonHFCPropagator* prop,Rule* rule){

    if(rule->head_size == 0) return this ; 

    const clingo_atom_t* head = rule->head ;
    const clingo_literal_t* body = rule->body ;
    size_t clause_size = rule->head_size + rule->body_size ;
    int index_clause = 0 ;


    std::string aux_r = createAux("r"+std::to_string(rule->id));
    std::vector<std::string> bodyConstraint;
    if(rule->head_size > 1){
        for(int i = 0 ; i < rule->head_size; ++i){
            clingo_atom_t atom = head[i];
            std::string p = createLiteralString(atom);            
            ++index_clause;

            bool ext = prop->component->external_atoms.find(atom) != prop->component->external_atoms.end();
            if(ext){
                bodyConstraint.push_back(negateLiteral(p));
            }else{
                std::string h = createHAtom(p);
                bodyConstraint.push_back(negateLiteral(h));
                std::string u = createUAtom(p);
                std::vector<std::string> implication = {u, negateLiteral(aux_r)};
                withASPConstraint(prop, implication);
            } 
        }
    }else{
        std::string head_str = createLiteralString(head[0]);
        std::string u = createUAtom(head_str);
        aux_r = u;
    }
    freshAtoms.emplace(aux_r);
    bodyConstraint.push_back(aux_r);

    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t literal = body[i];
        bool negative = literal < 0 ;
        bool ext = prop->component->external_atoms.find(std::abs(literal)) != prop->component->external_atoms.end() ;
        literal = std::abs(literal);
        std::string p = createLiteralString(literal);
        if (negative) p = negateLiteral(p) ;
        bodyConstraint.push_back(p);
        if(!ext && !negative){
            std::string u = createUAtom(p);
            bodyConstraint.push_back(negateLiteral(u));
        }
        ++index_clause;
    }
    withASPRule(prop, false, nullptr, &bodyConstraint);

    return this;
}

EncodingBuilder* UnfoundedEncodingBuilder::withWeightRule(NonHFCPropagator* prop, const WeightRule* rule){
    assert(rule->head_size == 1);
    clingo_atom_t aggr = rule->head[0];
    std::string aggr_str = createLiteralString(aggr);
    std::string u_aggr = createUAtom(aggr_str);
    std::string h_aggr = createHAtom(aggr_str);
    // std::vector<std::string> implication = {u_aggr, aggr_str};
    // withASPConstraint(prop, implication);

    std::string aggr_str_neg = negateLiteral(aggr_str);
    clingo_weight_t b = rule->get_lower_bound();
    std::vector<AggregateElement> elementsRightDirection ;
    // AggregateElement aggr_element = {.atomName = u_aggr, .weight = b};
    AggregateElement aggr_element = {.atomName = negateLiteral(h_aggr), .weight = b};
    elementsRightDirection.push_back(aggr_element);
    size_t n = rule->body_size ;
    
    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t l = rule->get_weighted_body()[i].literal;
        clingo_weight_t w = rule->get_weighted_body()[i].weight ;
        std::string name  = createLiteralString(l);
        bool ext = prop->component->external_atoms.find(std::abs(l)) != prop->component->external_atoms.end() ;
        if(l>0 && !ext){
            name = createHAtom(name);
        }else if(l < 0){
            name = negateLiteral(createLiteralString(std::abs(l)));
        }
        AggregateElement e = {.atomName = name, .weight = w};
        elementsRightDirection.push_back(e);
    }

    // Modelling one direction
    withASPSumRule(prop, false, nullptr, elementsRightDirection, b);
    // withASPSumConstraint(prop, elementsRightDirection, b);

    clingo_weight_t sum_w = 0 ; 
    for(int i = 0; i < n; ++i) sum_w += rule->get_weighted_body()[i].weight ; 

    std::vector<AggregateElement> elementsLeftDirection ;
    clingo_weight_t weight_left_dir = sum_w - b + 1 ; 
    // aggr_element = {.atomName = negateLiteral(u_aggr), .weight = weight_left_dir};
    aggr_element = {.atomName = h_aggr, .weight = weight_left_dir};
    elementsLeftDirection.push_back(aggr_element);
    for(int i = 0 ; i < rule->body_size; ++i){
        clingo_literal_t l = rule->get_weighted_body()[i].literal;
        std::string name ;
        bool ext = prop->component->external_atoms.find(std::abs(l)) != prop->component->external_atoms.end() ;
        if(l > 0){
            if(!ext){
                name = negateLiteral(createHAtom(createLiteralString(l))) ;
            }else{
                name = negateLiteral(createLiteralString(l));
            }
           
        }else{
            name = createLiteralString(std::abs(l));
        }
        clingo_weight_t w = rule->get_weighted_body()[i].weight ;
        AggregateElement e = {.atomName = name, .weight = w};
        elementsLeftDirection.push_back(e);
    }
    
    // Modelling the other direction
    withASPSumRule(prop, false, nullptr, elementsLeftDirection, weight_left_dir);
    // withASPSumConstraint(prop, elementsLeftDirection, weight_left_dir);

    return this ;
}
