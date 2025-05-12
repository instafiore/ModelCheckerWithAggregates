#pragma once
#include <clingo.h>
#include <vector>
#include <iostream>
#include "../utils/utility.h"
#include <ostream>
#include "../dipendency_graph/dependency_graph.h"
#include "../utils/utility.h"
#include "../utils/logger.h"


class NonHFCPropagator;

class InterpretationFunction: public SymmetricFunction<int>{

    int function_negative_lit(int value) const override { 
        if (value == NONE)  return value ;
        return 1 - value ;
    }

    InterpretationFunction(int N) : SymmetricFunction(N) {}
    static InterpretationFunction* instance ;

public:

    static InterpretationFunction* getInstance(int N = 0){
        // if(N>0) print("[size] N: ",N);
        if(instance == nullptr) instance = new InterpretationFunction(N) ;
        return instance ;
    }
    
};

inline InterpretationFunction* InterpretationFunction::instance = nullptr;

enum RULE_TYPE { NR, AGGR, DISJ, CHOICE, CONS } ;
class Rule{

public:

    size_t id;
    bool choice;
    clingo_atom_t *head = nullptr;
    size_t head_size; 
    clingo_literal_t *body = nullptr; 
    size_t body_size;
    RULE_TYPE type ;

    Rule(size_t id, bool choice, const clingo_atom_t* head, size_t head_size, const clingo_literal_t* body, size_t body_size)
    : id(id), 
    choice(choice),
    head_size(head_size),
    body_size(body_size) {
        if(head_size > 0 && head != NULL){
            // print("[size] head_size: ",head_size);
            this->head = new clingo_atom_t[head_size];
            std::copy(head, head + head_size, this->head);
        }

        if(body_size && body != NULL){
            // print("[size] body_size: ",body_size);
            this->body = new clingo_literal_t[body_size];
            std::copy(body, body + body_size, this->body);
        }
    }

    void setType(RULE_TYPE type){ this->type = type ; }
    virtual ~Rule(){
        if(head != nullptr) delete[] head;
        if(body != nullptr) delete[] body;
    }
    RULE_TYPE getType(){ return type;}
    bool isChoice() const { return choice; }
    bool isInHead(clingo_atom_t atom);
    virtual bool isInBPlus(clingo_atom_t atom);
    virtual bool isInBPlus(std::vector<clingo_atom_t> atoms);
    virtual bool isInBMinus(clingo_atom_t atom);
    virtual bool isTrueBody(const InterpretationFunction* I);
    virtual clingo_literal_t isFalseBody(const InterpretationFunction* I);
    virtual clingo_atom_t isTrueHead(const InterpretationFunction* I, const PerfectVector<clingo_atom_t>& X);
    
    virtual clingo_literal_t reasonHead(const InterpretationFunction* I, NonHFCPropagator* prop, bool &found);
    virtual clingo_literal_t reasonBody(const InterpretationFunction* I, NonHFCPropagator* prop, bool &found);
    
    virtual std::string toString() const;

};

typedef struct AggregateElement{
    std::string atomName ;
    int weight ;
};

class WeightRule: public Rule{ 

private:
    clingo_weight_t lower_bound;
    clingo_weighted_literal_t *weighted_body = nullptr;

public:
    WeightRule(size_t id, bool choice, clingo_atom_t const *head, size_t head_size, clingo_weight_t lower_bound, clingo_weighted_literal_t const *weighted_body, size_t weighted_body_size) : Rule(id, choice, head, head_size, NULL, weighted_body_size), lower_bound(lower_bound) {
        if(weighted_body_size > 0 && weighted_body != NULL){
            // print("[size] weighted_body_size: ",weighted_body_size);
            this->weighted_body = new clingo_weighted_literal_t[weighted_body_size];
            std::copy(weighted_body, weighted_body + weighted_body_size, this->weighted_body);
        }
    }
    clingo_weight_t get_lower_bound() const { return lower_bound; }
    const clingo_weighted_literal_t  *get_weighted_body() const { return weighted_body; }
    size_t get_weighted_body_size() const { return Rule::body_size; }
    bool isHeadUnfounded(const PerfectVector<clingo_atom_t>& unfoundedSet);
    int getSum(InterpretationFunction* I) const;
    void computeReason(InterpretationFunction* I, NonHFCPropagator* prop, PerfectVector<clingo_literal_t>* reason) const ;
    std::string toString() const override;
    ~WeightRule(){
        if(weighted_body != nullptr) delete[] weighted_body ;
    }
};

class Component{

    public:
        int id;
        std::unordered_set<clingo_atom_t> external_atoms;
        std::vector<clingo_atom_t> internal_atoms;
        std::vector<Rule*> allRules;
        std::vector<Rule*> rules;
        std::vector<Rule*> constraints;
        std::vector<WeightRule*> aggregateRules;
        unsigned countDefinedLits = 0;
        Component(int id) : id(id) {}
        int get_id() const { return id; }
        unsigned getSize() const { return internal_atoms.size() + external_atoms.size(); }
        void increaseCountDefinedLits() { countDefinedLits++; }
        unsigned getCountDefinedLits() const { return countDefinedLits; }
        void decreaseCountDefinedLits() { countDefinedLits--; }
        void add_rule(Rule* rule);
        // std::vector<Rule*> getAllRules() const { return allRules; }
        
        std::string toString() const;

        std::vector<Rule*> getRules() const { return rules; }
        std::vector<WeightRule*> get_aggregates() const { return aggregateRules; }
};

class Program{
private:
    DependencyGraph dependency_graph;
    std::vector<Rule*> allRules;
    std::vector<Component*> components;

    // static Program* instance;
    
public:

    std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames = new std::unordered_map<clingo_symbol_t, clingo_atom_t>();
    // std::unordered_map<clingo_literal_t, std::string>* plit2rep;
    std::unordered_map<std::string, clingo_literal_t>* rep2plit = new std::unordered_map<std::string, clingo_literal_t>(); // It is necessary
    std::unordered_map<clingo_literal_t, std::vector<clingo_literal_t>>* map_slit_plit = new std::unordered_map<clingo_literal_t, std::vector<clingo_literal_t>>();;
    std::unordered_map<clingo_literal_t, clingo_literal_t>* map_plit_slit = new std::unordered_map<clingo_literal_t, clingo_literal_t>() ;
    size_t nt;
    clingo_literal_t max_plit = 0 ;
    PerfectHash<clingo_atom_t>* map_atom_component = nullptr;
    clingo_control* ctl ;

    Program() {}
    // static Program* getInstance(){
    //     if(instance == nullptr) instance = new Program();
    //     return instance;
    // }
    void add_rule(Rule* rule);
    void init(clingo_control* ctl, clingo_propagate_init* ctl_init, bool print_b);
    ~Program(){
        for(auto& rule: allRules){
            if(rule) delete rule;
        }
        for(auto& component: components){
            assert(component != nullptr);
            // debug(INFO,"Component size: ", component->getSize());
            if(component != nullptr) delete component;
        }
        
        if(atomNames) delete atomNames ; 
        if(map_slit_plit) delete map_slit_plit ; 
        if(map_plit_slit) delete map_plit_slit ; 
        if(rep2plit) delete rep2plit ; 
        if(map_atom_component) delete map_atom_component;
    }

    // static void cleanup(){
    //     if(instance != nullptr) delete instance;
    // }   

    void computeNonHFCComponents();
    std::vector<Component*>& getComponents() { 
        return components; 
    }

    std::string toString () const ;
};


