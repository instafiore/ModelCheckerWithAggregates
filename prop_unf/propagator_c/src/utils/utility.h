#pragma once
#include <clingo.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "../settings.h"
#include <assert.h>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <iostream>
#include <clingo.h>
#include <sstream>
#include <cassert>
#include <unordered_set>
#include <unordered_set>
#include <chrono>


enum Operator { G = 0, GE, L, LE };

class PropagatorAbstraction;
class Component ;
class Program;
typedef struct StatsStruct;
typedef struct AggregateElement;

template<typename T>
class PerfectVector ;

template<typename... Args>
void debug_print(const Args&... args) {
    std::ostringstream oss;
    (oss << ... << args); 
    std::cerr << oss.str() << std::endl;
}

template<typename... Args>
void print(const Args&... args) {
    std::ostringstream oss;
    (oss << ... << args); 
    std::cout << oss.str() << std::endl;
}

bool equals(const clingo_literal_t& l1, const clingo_literal_t& l2);
std::unordered_map<std::string, std::string> init_param(int argc, char const *argv[]);
std::string cat(const std::string &filename);
void handle_error(bool success);
const std::string clingo_error_code_to_string(clingo_error_t code);
bool from_model_to_symbols(clingo_model_t const *model, std::vector<clingo_symbol_t>& symbols);
void printAnswerset(const std::vector<clingo_symbol_t>& answerset, size_t i = 0);
void printAnswersetWithNegativeLiterals(std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, const std::vector<clingo_symbol_t>& answerset, size_t i = 0);
bool solveMain(std::unordered_map<std::string, std::string>& params, Program &program, clingo_control_t *ctl, clingo_solve_result_bitset_t *result, clingo_literal_t *assumptions, size_t assumptions_size);
void setExitCode(int exitCode);
int solve(std::string programName, clingo_control_t *ctl, const std::vector<clingo_literal_t>& assumptions, std::unordered_set<clingo_atom_t>* answerset = nullptr, const Program* prog = nullptr);
inline int not_(clingo_literal_t literal) { return -literal; }
void print_propagate(PropagatorAbstraction* prop, const clingo_literal_t *changes, size_t size, clingo_propagate_control_t *control, int dl, bool force_print);
void print_reason(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames,  const std::vector<clingo_literal_t>& R, clingo_literal_t lit, bool force_print);
void print_reason(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames,  const std::vector<clingo_literal_t>& R, bool force_print);
void print_undo(PropagatorAbstraction* prop, const clingo_literal_t *changes, size_t size, clingo_propagate_control_t *control, int dl, int td, bool force_print);
bool rule_callback(bool choice, clingo_atom_t const *head, size_t head_size, clingo_literal_t const *body, size_t body_size, void *data);
bool weight_rule_callback(bool choice, clingo_atom_t const *head, size_t head_size, clingo_weight_t lower_bound, clingo_weighted_literal_t const *body, size_t body_size, void *data);
std::string getName(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, clingo_literal_t plit);
std::string atomNames_to_string(const std::unordered_map<clingo_symbol_t, clingo_literal_t>* atomNames);
void print_stats(bool force=false);
void print_stats_old(bool force = false);
bool print_clingo_statistics(const clingo_statistics_t *stats, uint64_t key, int depth);
void print_prefix(int depth);
std::string from_symbol_to_string(clingo_symbol_t symbol);
int getErrorCodeFromResult(clingo_solve_result_bitset_t solve_ret, std::string programName = "");
bool isNegative(std::string literal_str);
clingo_literal_t fromRep2plitProgram(std::string lit_rep);
bool isPrimeOracleLiteral(std::string rep);
bool isAuxOracleLiteral(std::string rep);
bool isHLiteral(std::string rep);
bool isULiteral(std::string rep);
bool isAuxRAtom(std::string rep);
std::string negateLiteral(std::string literal);
std::string negateAtomAnswerset(std::string atom);
std::string removeNegation(std::string literal_str);
std::string createHead(bool choice, const std::vector<std::string>* head);
std::string createBody(const std::vector<std::string>* body);
std::string createSum(const std::vector<AggregateElement> elements, int lb, Operator op);
std::string createLiteralString(clingo_literal_t literal);
std::string createPrime(std::string atom);
std::string createHAtom(std::string atom);
std::string createUAtom(std::string atom);
std::string createAux(std::string literal);
std::chrono::time_point<std::chrono::high_resolution_clock> start_timer();
void display_end_timer(const std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::string name);
std::chrono::duration<double> elapsed_time(const std::chrono::time_point<std::chrono::high_resolution_clock>& start);
std::string indent(int depth);

// template<typename T>
// void swap(T& a, T& b);

// void set_no_ufs_check(clingo_control_t *ctl);
void logger(clingo_warning_t code, char const *message, void *data);
void register_propagator(clingo_control_t *ctl, clingo_propagator_t prop, Component* component, std::unordered_map<std::string, std::string>& params, std::vector<PropagatorAbstraction*> &propagators);

template <typename K, typename V>
void update_map_value_vector(std::unordered_map<K, std::vector<V>>& umap, K key, V value);
template <typename K, typename V>
std::vector<V> get_map_value_vector(std::unordered_map<K, std::vector<V>>& umap, K key);
template <typename K, typename V>
std::vector<V> get_map_value_vector(std::map<K, std::vector<V>>& umap, K key);
template< typename T>
void extend_vector(std::vector<T>& to_extend, const std::vector<T>& input, int i = 0, int j = -1);
template <typename T>
std::string vector_to_string(const std::vector<T>& vec, std::string name = "");
template <typename T>
std::string vector_to_string_name(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::vector<T>& vec, std::string name = "");
template <typename T>
std::string array_to_string(const T* vec, size_t size, std::string name = "");
template <typename T1, typename T2>
std::string unordered_map_to_string(const std::unordered_map<T1,T2>& umap, std::string name );
template <typename T>
std::string unordered_set_to_string(const std::unordered_set<T>& uset, std::string name = "");
template <typename T>
std::string unordered_set_to_string_name(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::unordered_set<T>& uset, std::string name = "" );


template <typename V>
class PerfectHash {

public:
    // Constructor that initializes the hash table
    PerfectHash(int N, V default_value = V()): N(N), values(2*N,default_value){}

    // Getter for index access
    virtual V get(int key) const;

    // Setter for index access
    virtual void set(const int &key, const V& value);
    
    // V get_set_default(int lit, const V& invalid_value);

    virtual ~PerfectHash(){}

protected:
    std::vector<V> values;  
private:
    int N;                 
};

template <typename T>
class SymmetricFunction {
private:
    std::vector<T> data_structure;

protected:
    T NONE  ;   
    virtual T function_negative_lit(T value) const  {
        return value ;
    } 
    

public:

    size_t N;

    // Constructor
    explicit SymmetricFunction(size_t N, T NONE = SETTINGS::NONE) : 
        data_structure(N,NONE), NONE(NONE), N(N) {}

    
    // Getter (accessor)
    virtual T get(int lit) const {
        int i = std::abs(lit);
        auto value = data_structure[i];
        if (lit < 0) value = function_negative_lit(value);
        return value;
    }

    // Setter (mutator)
    void set(int lit, T value) {
        int i = std::abs(lit);
        if (lit < 0) value = function_negative_lit(value);
        data_structure[i] = value;
    }

    virtual ~SymmetricFunction(){}
};

class PerfectSet: public PerfectHash<int>{
    private:
        int cont = 0 ;
    public:
        PerfectSet(int N): PerfectHash(N, -1){}
    
        int get(int key) const override{
            int value = PerfectHash::get(key);
            bool res = value == cont;
            return res;
        }
    
        void set(const int& key, const int& value) override{
            if(value == true) PerfectHash::set(key, cont);
            else if(value == false) PerfectHash::set(key, cont-1);
            else assert(false) ;
        }
    
        inline constexpr void clear(){ ++cont ; }

        virtual ~PerfectSet(){}

};


template<typename T>
class PerfectVector {

private:
    PerfectSet checker ;
    std::vector<T> _data_ ;

public:
    PerfectVector(int N): checker(N){} 
    void add(T lit);
    void clear(); 
    bool has(T lit) const ;
    const std::vector<T>& data() const { return _data_; }

    // Vector-like methods
    T& operator[](size_t index) { return _data_[index]; }
    const T& operator[](size_t index) const { return _data_[index]; }
    
    void push_back(const T& value) { add(value); }
    void push_back(T&& value) { add(std::move(value)); }
    T& back() { return _data_.back(); }

    void pop_back() {
        if (!_data_.empty()) {
            checker.set(_data_.back(), false);
            _data_.pop_back();
        }
    }
    
    size_t size() const { return _data_.size(); }
    bool empty() const { return _data_.empty(); }
    
    void reserve(size_t new_cap) { _data_.reserve(new_cap); }
    void shrink_to_fit() { _data_.shrink_to_fit(); }
    
    auto begin() { return _data_.begin(); }
    auto end() { return _data_.end(); }
    auto begin() const { return _data_.begin(); }
    auto end() const { return _data_.end(); }
    
    auto rbegin() { return _data_.rbegin(); }
    auto rend() { return _data_.rend(); }
    auto rbegin() const { return _data_.rbegin(); }
    auto rend() const { return _data_.rend(); }
};
    


#include "utility.tpp"
