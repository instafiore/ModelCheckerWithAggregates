#pragma once
#include <iostream>
#include <clingo.h>
#include <string>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <limits>
#include <unordered_map>


class Checker{

private: 
    std::string encoding_path;
    std::string instance_path;
    std::unordered_map<std::string, std::string>& params;
    clingo_solve_result_bitset_t solve_ret;
    std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames;
    clingo_control_t *ctl ;
    bool __check__(std::vector<clingo_symbol_t>* model, bool sat);
    Checker(std::string encoding_path, std::string instance_path, std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, std::unordered_map<std::string, std::string>& params);

public:
    static Checker* instance;
    static void init(std::string encoding_path, std::string instance_path, std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, std::unordered_map<std::string, std::string>& params);
    bool checkSat(std::vector<clingo_symbol_t>& model);
    bool checkUnsat();
    static Checker* getInstance();
    static void cleanup();
    ~Checker();

};