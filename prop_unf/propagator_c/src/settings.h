#pragma once
#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <stdexcept>
#include <limits>
#include <filesystem>


namespace SETTINGS {
const int NONE = std::numeric_limits<int>::max() ;
const int SAT_CODE = 10;
const int UNSAT_CODE = 20;
const int ERROR_CODE = 1;
inline int EXIT_CODE = SETTINGS::NONE ;
const std::string TRUE_STR = "True" ;
const std::string FALSE_STR = "False" ;
const char SEPARATOR_ASSUMPTIONS = ':';
const char NOT = '~';
const std::string NONE_STR = "None";
const std::regex regexNegativeLiteral(R"(not\s(.+))");
const std::regex regexPrimeOracleOptional(R"(a(\d+)p?)");
const std::regex regexAux(R"(aux_(\d+))");
const std::regex regexAuxR(R"(aux_r(\d+))");
const std::regex regexH(R"(h_a(\d+))");
const std::regex regexU(R"(u_a(\d+))");
const std::regex regexPrimeOracle(R"(a(\d+)p)");
inline std::filesystem::path ROOT;
const std::string ENCODING = "encoding";
const std::string INSTANCE = "instance";
const std::string LOGFILE = "logfile";
const std::string NMODELS = "num_models";
const std::string TIMELIMIT = "time_limit";
}