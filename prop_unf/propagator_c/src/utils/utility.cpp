#include "utility.h"
#include "logger.h"
#include "../propagator/propagator_abstraction.h"
#include <clingo.h>
#include "../model/checker.h"
#include "stats.h"

bool equals(const clingo_literal_t& l1, const clingo_literal_t& l2){
    if(l1 == SETTINGS::NONE || l2 == SETTINGS::NONE)
        return l1 == SETTINGS::NONE && l2 == SETTINGS::NONE ;
    return abs(l1) == abs(l2) ;
}

std::unordered_map<std::string, std::string> init_param(int argc, char const *argv[]) {

    std::unordered_map<std::string, std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        size_t pos = arg.find('=');
        if (pos != std::string::npos && arg[0] == '-') {
            std::string key = arg.substr(1, pos - 1); 
            std::string value = arg.substr(pos + 1); 
            args[key] = value;
        } else if (arg[0] == '-'){
            std::string key = arg.substr(1);
            args[key] = SETTINGS::TRUE_STR;
        } else {
            std::cerr << "Invalid argument format: " << arg << "\n";
        }
    }

    return args;
}

int getErrorCodeFromResult(clingo_solve_result_bitset_t solve_ret, std::string programName){
    
    int res = ERROR ;
    if (solve_ret & clingo_solve_result_satisfiable) {
        debug(INFO, "result ",programName,": SAT");
        res = SETTINGS::SAT_CODE;
    } 
    else if (solve_ret & clingo_solve_result_unsatisfiable) {
        debug(INFO,"result ",programName,": UNSAT");
        res = SETTINGS::UNSAT_CODE;
    } 
    else if (solve_ret & clingo_solve_result_exhausted) {
        debug(ERROR,"ERROR ",programName,": Search space exhausted.");
        res = SETTINGS::ERROR_CODE;
    }
    else if (solve_ret & clingo_solve_result_interrupted) {
        debug(ERROR,"timeout ",programName,": Solving was interrupted.");
        res = SETTINGS::ERROR_CODE;
    }else {
        debug(ERROR,"Unexpected solve result ",programName,".");
        res = SETTINGS::ERROR_CODE;
    } 
    return res ;
}


std::string cat(const std::string &filename) {
    std::ifstream file(filename); 
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return SETTINGS::NONE_STR;
    }

    std::ostringstream oss ;
    std::string line;
    while (std::getline(file, line)) { 
        oss << line << '\n';    
    }

    file.close(); 
    return oss.str() ;
}

void handle_error(bool success) {
    char const *error_message;
    if (!success) {
        if (!(error_message = clingo_error_message())) { error_message = "generic error, not recognized from clingo"; }
        clingo_error_t code = clingo_error_code();
        std::string code_str = clingo_error_code_to_string(code);
        std::stringstream ss;
        ss << "Clingo error: " << error_message << " with code: " << code_str << '\n';
        debug(ERROR, ss.str());
        setExitCode(SETTINGS::ERROR_CODE);
    }
}

void setExitCode(int exitCode){
    if(exitCode == SETTINGS::ERROR_CODE) SETTINGS::EXIT_CODE = SETTINGS::ERROR_CODE ;
    if(SETTINGS::EXIT_CODE == SETTINGS::ERROR_CODE){
        print("ERROR");
        exit(SETTINGS::ERROR_CODE);
    } ;
    assert(SETTINGS::EXIT_CODE == SETTINGS::NONE);
    SETTINGS::EXIT_CODE = exitCode ;
}

void set_no_ufs_check(clingo_control_t *ctl) {
    clingo_configuration_t *config;
    clingo_id_t root_key, ufs_check_key;

    // Get configuration
    if (!clingo_control_configuration(ctl, &config)) {
        debug(ERROR, "Failed to get configuration");
    }

    // Get root key
    if (!clingo_configuration_root(config, &root_key)) {
        debug(ERROR, "Failed to get root key");
    }

    // Get the key for `ufs_check`
    if (!clingo_configuration_map_at(config, root_key, "ufs_check", &ufs_check_key)) {
        debug(ERROR, "Failed to get ufs_check key");
    }

    // Set `ufs_check` to false (disabling unfounded set check)
    if (!clingo_configuration_value_set(config, ufs_check_key, "false")) {
        debug(ERROR, "Failed to set ufs_check option");
    }
}

void logger(clingo_warning_t code, char const *message, void *data) {
    std::stringstream ss;
    ss << "Clingo warning [" << code << "]: " << message << std::endl;
    debug(CLINGO, ss.str());
}

void register_propagator(clingo_control_t *ctl, clingo_propagator_t prop, Component* component, std::unordered_map<std::string, std::string>& params, std::vector<PropagatorAbstraction*> &propagators){

    PropagatorAbstraction* propagator = new PropagatorAbstraction(component, params);
    handle_error(clingo_control_register_propagator(ctl, &prop, propagator, false));
    propagators.push_back(propagator);
}

const std::string clingo_error_code_to_string(clingo_error_t code) {
    switch (code) {
        case clingo_error_bad_alloc:
            return "memory could not be allocated";
        case clingo_error_unknown:
            return "errors unrelated to clingo";
        case clingo_error_success:
            return "successful API calls";
        case clingo_error_runtime:
            return "errors only detectable at runtime like invalid input";
        case clingo_error_logic:
            return "wrong usage of the clingo API";
        default:
            return "unknown_error code: ";
    }
}

bool from_model_to_symbols(clingo_model_t const *model, std::vector<clingo_symbol_t>& symbols) {
    bool ret = true;
    clingo_symbol_t *atoms = NULL;
    size_t atoms_n;
    clingo_symbol_t const *it, *ie;
    char *str = NULL;
    size_t str_n = 0;
   
    // determine the number of (shown) symbols in the model
    if (!clingo_model_symbols_size(model, clingo_show_type_shown, &atoms_n)) { goto error; }
   
    // allocate required memory to hold all the symbols
    if (!(atoms = (clingo_symbol_t*)malloc(sizeof(*atoms) * atoms_n))) {
      clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for atoms");
      goto error;
    }
   
    // retrieve the symbols in the model
    if (!clingo_model_symbols(model, clingo_show_type_shown, atoms, atoms_n)) { goto error; }
   
    // printf("Answer set %d {",i);
   
    for (it = atoms, ie = atoms + atoms_n; it != ie; ++it) {
      size_t n;
      char *str_new;
   
      // determine size of the string representation of the next symbol in the model
      if (!clingo_symbol_to_string_size(*it, &n)) { goto error; }
   
      if (str_n < n) {
        // allocate required memory to hold the symbol's string
        if (!(str_new = (char*)realloc(str, sizeof(*str) * n))) {
          clingo_set_error(clingo_error_bad_alloc, "could not allocate memory for symbol's string");
          goto error;
        }
   
        str = str_new;
        str_n = n;
      }
   
      // retrieve the symbol's string
      if (!clingo_symbol_to_string(*it, str, n)) { goto error; }

      symbols.push_back(*it);
   
    //   it+1 != ie ? printf("%s, ", str, n) : printf(" %s", str);
  
    }
   
    goto out;
   
  error:
    ret = false;
   
  out:
    if (atoms) { free(atoms); }
    if (str)   { free(str); }
   
    return ret;
  }

  
bool solveMain(std::unordered_map<std::string, std::string>& params, Program &program, clingo_control_t *ctl, clingo_solve_result_bitset_t *result, clingo_literal_t *assumptions = nullptr, size_t assumptions_size = 0) {
    bool ret = true;
    clingo_solve_handle_t *handle;
    clingo_model_t const *model;

    // get a solve handle
    handle_error(clingo_control_solve(ctl, clingo_solve_mode_yield, assumptions, assumptions_size, NULL, NULL, &handle));
    // loop over all models
    size_t i = 1 ;
    bool finished ; 
    bool toCheck = params.find("check")->second == SETTINGS::TRUE_STR ; 
    bool successCheck = true; 
    while (true) {
        handle_error(clingo_solve_handle_resume(handle));
        //   clingo_solve_handle_wait(handle, 1.0, &finished);
        handle_error(clingo_solve_handle_model(handle, &model));
        // print the model
        if (model) { 
            std::vector<clingo_symbol_t> answerset ; 
            from_model_to_symbols(model,answerset);
            // printAnswerset(answerset,i);
            printAnswersetWithNegativeLiterals(*program.atomNames,answerset,i);
            // debug(INFO, "params.find('check')->second: ",params.find("check")->second, "", params.find("check")->second == SETTINGS::TRUE_STR, " ", SETTINGS::TRUE_STR);
            if(params.find("check")->second == SETTINGS::TRUE_STR){
                debug(INFO, "Checking...");
                bool check = Checker::getInstance()->checkSat(answerset);
                if(!check){
                    print("Check did not pass !! X");
                    debug(ERROR, "Check did not pass !! X");
                    successCheck = false ;
                }else{
                    print("Check passed! V");
                    debug(INFO, "Check passed! V");
                }
            }
            // debug("result check: ",check);
        }
        // stop if there are no more models
        else break; 
        ++i ;
    }

    if(!successCheck){
        // ERROR
        setExitCode(SETTINGS::ERROR_CODE);
    }

    handle_error(clingo_solve_handle_get(handle, result));
    
    return clingo_solve_handle_close(handle) && ret;
}


std::chrono::time_point<std::chrono::high_resolution_clock> start_timer(){
    return std::chrono::high_resolution_clock::now(); 
}

void display_end_timer(const std::chrono::time_point<std::chrono::high_resolution_clock>& start, std::string name){
    auto end = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double> elapsed = (end - start);
    debug(STATS,name," time: ", elapsed.count(), "s");
}

std::chrono::duration<double> elapsed_time(const std::chrono::time_point<std::chrono::high_resolution_clock>& start){
    auto end = std::chrono::high_resolution_clock::now(); 
    return end - start;
}



void printAnswerset(const std::vector<clingo_symbol_t>& answerset, size_t i){
    printf("Answer set %d {",i);

    for(int i = 0; i < answerset.size(); ++i){
        const auto& atom =  answerset[i];
        std::string name = from_symbol_to_string(atom);
        i < answerset.size() - 1 ? printf("%s, ", name.c_str()) : printf(" %s", name.c_str());
    }

    printf("}\n");
}

void printAnswersetWithNegativeLiterals(std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, const std::vector<clingo_symbol_t>& answerset, size_t i){
    printf("Answer set %d {",i);

    std::vector<std::string> answersetWithNegativeLitearls ;
    for(const auto&[sym, plit] : atomNames){
        std::string atomName = from_symbol_to_string(sym);
        if(std::find(answerset.begin(), answerset.end(), sym) == answerset.end()){
            // false literal
            atomName = negateAtomAnswerset(atomName);
        }
        answersetWithNegativeLitearls.push_back(atomName);
    }

    size_t n = answersetWithNegativeLitearls.size() ;
    for(int i = 0; i < n; ++i){
        const auto& literalName =  answersetWithNegativeLitearls[i];
        // i < n - 1 ? printf("%s, ", literalName.c_str()) : printf(" %s", literalName.c_str());
        i < n - 1 ? printf("%s\t", literalName.c_str()) : printf("%s", literalName.c_str());
        // printf("%s\t", literalName.c_str());
    }

    printf("}\n");
}

std::string negateLiteral(std::string literal){
    return "not " + literal;
}

std::string negateAtomAnswerset(std::string atom){
    return "~" + atom;
}

std::string createLiteralString(clingo_literal_t literal){ return "a" + std::to_string(literal);}
std::string createPrime(std::string atom){ return atom + "p"; }
std::string createAux(std::string literal){ return"aux_" + literal;}
std::string createHAtom(std::string atom){ return "h_" + atom;}
std::string createUAtom(std::string atom){ return "u_" + atom;}


std::string createHead(bool choice, const std::vector<std::string>* head){
    std::string str = "";
    if(head != nullptr){
        if (choice) str += "{";
        for(size_t i = 0; i < head->size(); i++){
            std::string del = choice ? ";" : "|";
            str += (*head)[i] + (i < head->size() - 1 && head->size() > 1 ? del : "");    
        }
        if (choice) str += "}";
    }
    return str ;
}

std::string createBody(const std::vector<std::string>* body){
    std::string str = "";
    if(body != nullptr && body->size() > 0){ 
        str +=":- ";
        for(size_t i = 0; i < body->size(); i++){
            str += (*body)[i] + (i < body->size() - 1 ? ", " : "");
        }
    }
    return str ;
}

std::string createSum(const std::vector<AggregateElement> elements, int lb, Operator op){
    std::string operator_s = op <= 1 ?  ">" : "<"; 
    operator_s += op % 2 == 0 ? "" : "=";
    std::string sum = "#sum{ ";
    for(int i = 0 ; i < elements.size(); ++i){
        const auto& e = elements[i];
        sum += std::to_string(e.weight) + "," + removeNegation(e.atomName) + ":" + e.atomName + (i < elements.size()-1 ? "; " : "");
    } 
    sum +=  "} " + operator_s + " " + std::to_string(lb) ;
    
    return sum ;
}

int solve(std::string programName, clingo_control_t *ctl, const std::vector<clingo_literal_t>& assumptions, std::unordered_set<clingo_atom_t>* answerset, const Program* prog){

    clingo_solve_handle_t *handle;
    clingo_model_t const *model;
    clingo_solve_result_bitset_t result; 

    if(answerset) answerset->clear();
    handle_error(clingo_control_solve(ctl, clingo_solve_mode_yield, assumptions.data(), assumptions.size(), NULL, NULL, &handle));
    // handle_error(clingo_control_solve(controlOracle, clingo_solve_mode_yield, nullptr, 0, NULL, NULL, &handle));

    clingo_solve_handle_model(handle, &model);
    handle_error(clingo_solve_handle_get(handle, &result)); 
    int exitCode = getErrorCodeFromResult(result, programName);

    if(exitCode == SETTINGS::SAT_CODE && answerset){

        size_t sizeAnswerset;
        handle_error(clingo_model_symbols_size(model, clingo_show_type_shown, &sizeAnswerset));
        // print("[size] sizeAnswerset: ", sizeAnswerset);
        clingo_symbol_t *symbolsAnswerset = new clingo_symbol_t[sizeAnswerset];
        handle_error(clingo_model_symbols(model, clingo_show_type_shown, symbolsAnswerset, sizeAnswerset));

        for (size_t i = 0; i < sizeAnswerset; ++i) {
            clingo_symbol_t sym = symbolsAnswerset[i] ;
            clingo_atom_t atomOracle = (*prog->atomNames)[sym];
            answerset->insert(atomOracle);
        }

        delete[] symbolsAnswerset;  
    }
    handle_error(clingo_solve_handle_close(handle));
    return exitCode ;
}

std::string from_symbol_to_string(clingo_symbol_t symbol){
    size_t symbol_size;
    handle_error(clingo_symbol_to_string_size(symbol, &symbol_size));
    char symbol_str_c[symbol_size]; 
    handle_error(clingo_symbol_to_string(symbol, symbol_str_c, symbol_size));
    std::string symbol_str = std::string(symbol_str_c);
    return symbol_str ;
}

void print_propagate(PropagatorAbstraction* prop, const clingo_literal_t *changes, size_t size, clingo_propagate_control_t *control, int dl, bool force_print = false){

    #ifndef LOGGER_ACTIVE
        return ;
    #endif

    bool debug_b = true ;
    #ifdef NODEBUG
        debug_b = false;
    #endif
    if (not force_print and not debug_b) return ;


    int td ;
    td = clingo_propagate_control_thread_id(control) ; 
    std::string changes_str ;
    
    
    changes_str = prop->compute_changes_str(changes, size, td) ;
    
    const clingo_assignment_t *assignment = clingo_propagate_control_assignment(control);
    clingo_literal_t decision_slit ;
    assert(assignment != NULL);
    handle_error(clingo_assignment_decision(assignment, dl, &decision_slit));

    clingo_literal_t plit = 0 ;
    bool aux = false ;
    if (decision_slit != 1){
        if(prop->prog->map_slit_plit->find(decision_slit) == prop->prog->map_slit_plit->end()){
            aux = true ;
        }else {
            plit = (*prop->prog->map_slit_plit)[decision_slit][0];
        }
    }

    std::string decision_literal_name ; 
    if(!aux)
        decision_slit != 1 ? decision_literal_name = getName(prop->prog->atomNames, plit) : decision_literal_name = "from facts" ;
    else{
        decision_literal_name = "aux_"+ std::to_string(std::abs(decision_slit)) ;
        if(decision_slit < 0 ) decision_literal_name = negateLiteral(decision_literal_name);
    }
    debugf(DEBUG, "ID: ",prop->component->get_id()," [", decision_literal_name,", ",dl,"] propagate ", changes_str," td: ", td);
}

void print_reason(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::vector<clingo_literal_t>& R, clingo_literal_t lit = 0, bool force_print = false){
    #ifndef LOGGER_ACTIVE
        return ;
    #endif
    
    std::string reason_name = "Reason";
    if(lit != 0) std::string reason_name = "Reason("+std::to_string(lit)+") ";
    debug(DEBUG, vectorToString(R, reason_name));

}

void print_prefix(int depth) {
    for (int i = 0; i < depth; ++i) {
      printf("  ");
    }
  }

std::string indent(int depth) {
    std::string ind = "";
    for (int i = 0; i < depth; ++i) {
      ind += "\t" ;
    }
    return ind ; 
}

#ifdef LOGGER_ACTIVE
// recursively print the statistics object
bool print_clingo_statistics(const clingo_statistics_t *stats, uint64_t key, int depth) {
    #ifndef STATSCLINGO
        return true;
    #endif
    bool ret = true;
    clingo_statistics_type_t type;
   
    // get the type of an entry and switch over its various values
    if (!clingo_statistics_type(stats, key, &type)) { goto error; }
    switch ((enum clingo_statistics_type_e)type) {
      // print values
      case clingo_statistics_type_value: {
        double value;
   
        // print value (with prefix for readability)
        // print_prefix(depth);
        if (!clingo_statistics_value_get(stats, key, &value)) { goto error; }
        // printf("%g\n", value);
        Logger::getInstance()->log(STATS, indent(depth), value);
  
        break;
      }
   
      // print arrays
      case clingo_statistics_type_array: {
        size_t size;
   
        // loop over array elements
        if (!clingo_statistics_array_size(stats, key, &size)) { goto error; }
        for (size_t i = 0; i < size; ++i) {
          uint64_t subkey;
   
          // print array offset (with prefix for readability)
          if (!clingo_statistics_array_at(stats, key, i, &subkey)) { goto error; }
        //   print_prefix(depth);
        //   printf("%zu:\n", i);
          Logger::getInstance()->log(STATS,indent(depth), i);
   
          // recursively print subentry
          if (!print_clingo_statistics(stats, subkey, depth+1)) { goto error; }
        }
        break;
      }
   
      // print maps
      case clingo_statistics_type_map: {
        size_t size;
   
        // loop over map elements
        if (!clingo_statistics_map_size(stats, key, &size)) { goto error; }
        for (size_t i = 0; i < size; ++i) {
          char const *name;
          uint64_t subkey;
   
          // get and print map name (with prefix for readability)
          if (!clingo_statistics_map_subkey_name(stats, key, i, &name)) { goto error; }
          if (!clingo_statistics_map_at(stats, key, name, &subkey)) { goto error; }
        //   print_prefix(depth);
        //   printf("%s:\n", name);
            Logger::getInstance()->log(STATS,indent(depth), name);
   
          // recursively print subentry
          if (!print_clingo_statistics(stats, subkey, depth+1)) { goto error; }
        }
      }
   
      // this case won't occur if the statistics are traversed like this
      case clingo_statistics_type_empty: { goto out; }
    }
   
    goto out;
  error:
    ret = false;
  out:

    return ret;
}

void print_stats_old(bool force) {

    const StatsStruct& stats = Logger::getInstance()->stats;

    if((stats.numberOfChecks == 0 || stats.numberOfChecks % stats.freshRate != 0) && !force) return ;

    Logger::getInstance()->log(STATS, "Stats Report:");

    // Length stats
    // Logger::getInstance()->log(STATS, "\tTotal Length Reasons: ", stats.totalLengthReasons);
    Logger::getInstance()->log(STATS, "\tMax Length Reason: ", stats.maxLengthReason);
    Logger::getInstance()->log(STATS, "\tMin Length Reason: ", stats.minLengthReason);

    // Time Reason stats
    Logger::getInstance()->log(STATS, "\tTotal Time Reason: ", stats.totalTimeReason);
    Logger::getInstance()->log(STATS, "\tMax Time Reason: ", stats.maxTimeReason);
    Logger::getInstance()->log(STATS, "\tMin Time Reason: ", stats.minTimeReason);
    Logger::getInstance()->log(STATS, "\tNumber of Reasons: ", stats.numberOfReason);
    if (stats.numberOfReason > 0) {
        float meanTimeReason = static_cast<float>(stats.totalTimeReason) / stats.numberOfReason;
        Logger::getInstance()->log(STATS, "\tMean Time Reason: ", meanTimeReason);
    }

    // Time Checker stats
    Logger::getInstance()->log(STATS, "\tTotal Time Checker: ", stats.totalTimeChecker);
    Logger::getInstance()->log(STATS, "\tMax Time Checker: ", stats.maxTimeChecker);
    Logger::getInstance()->log(STATS, "\tMin Time Checker: ", stats.minTimeChecker);
    Logger::getInstance()->log(STATS, "\tNumber of Checks: ", stats.numberOfChecks);
    if (stats.numberOfChecks > 0) {
        float meanTimeChecker = static_cast<float>(stats.totalTimeChecker) / stats.numberOfChecks;
        Logger::getInstance()->log(STATS, "\tMean Time Checker: ", meanTimeChecker);
    }

}

void print_stats(bool force){

    if(!HandleStats::getInstance()->canPrint() && !force) return ;
    Logger::getInstance()->log(STATS, HandleStats::getInstance()->toString());
}

#endif

void print_reason(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, const std::vector<clingo_literal_t>& R, bool force_print = false){
    print_reason(atomNames, R, 0, force_print);
}

void print_undo(PropagatorAbstraction* prop, const clingo_literal_t *changes, size_t size, clingo_propagate_control_t *control, int dl, int td, bool force_print = false){
  
    #ifndef LOGGER_ACTIVE
        return ;
    #endif

    bool debug_b = true ;
    #ifdef NODEBUG
        debug_b = false;
    #endif
    if (not force_print and not debug_b) return ;

    std::string changes_str ;
    changes_str = prop->compute_changes_str(changes, size, td) ;
    debugf(DEBUG,"dl: ",dl," undo ", changes_str," thread_id: ", td);
}

size_t autoIncrementId = 0 ;

bool rule_callback(bool choice, clingo_atom_t const *head, size_t head_size, clingo_literal_t const *body, size_t body_size, void *data) {
    Program* program = (Program*)data;
    
    // print("r choice ",choice," head_size: ",head_size, " body_size: ",body_size);
    Rule* rule = new Rule(autoIncrementId++, choice, head, head_size, body, body_size);
    program->add_rule(rule);
    return true;
}

bool weight_rule_callback(bool choice, clingo_atom_t const *head, size_t head_size, clingo_weight_t lower_bound, clingo_weighted_literal_t const *body, size_t body_size, void *data){
    Program* program = (Program*)data;
    // print("w choice ",choice," head_size: ",head_size, " body_size: ",body_size);
    WeightRule* rule = new WeightRule(autoIncrementId++, choice, head, head_size, lower_bound, body, body_size);
    program->add_rule(rule);

    return true;
}  

bool isNegative(std::string literal_str){
    std::regex negative_pattern("not (.+)");
    std::smatch match;
    return std::regex_match(literal_str, match, negative_pattern);
}

std::string removeNegation(std::string literal_str){
    std::regex negative_pattern("not (.+)");
    std::smatch match;
    if(std::regex_match(literal_str, match, negative_pattern)){
        return match[1];
    }else return literal_str ; 
}

bool isPrimeOracleLiteral(std::string rep){
    return std::regex_match(rep, SETTINGS::regexPrimeOracle);
}

bool isAuxOracleLiteral(std::string rep){
    return std::regex_match(rep, SETTINGS::regexAux);
}

bool isAuxRAtom(std::string rep){
    return std::regex_match(rep, SETTINGS::regexAuxR);
}


bool isHLiteral(std::string rep){
    return std::regex_match(rep, SETTINGS::regexH);
}

bool isULiteral(std::string rep){
    return std::regex_match(rep, SETTINGS::regexU);
}


clingo_literal_t fromRep2plitProgram(std::string lit_rep){
    
    std::smatch match1 ;
    std::smatch match2 ;
    bool negative = false ;
    if (std::regex_match(lit_rep, match1, SETTINGS::regexNegativeLiteral)){
        negative = true ;
        lit_rep = match1[1];
    }

    if (std::regex_match(lit_rep, match2, SETTINGS::regexPrimeOracleOptional)){
        lit_rep = match2[1];
    }else if(std::regex_match(lit_rep, match2, SETTINGS::regexAux)){
        lit_rep = match2[1];
    }else if(std::regex_match(lit_rep, match2, SETTINGS::regexH)){
        lit_rep = match2[1];
    }else if(std::regex_match(lit_rep, match2, SETTINGS::regexU)){
        lit_rep = match2[1];
    }else if(std::regex_match(lit_rep, match2, SETTINGS::regexAuxR)){
        // No plit program associated
        return 0 ;
    }
    else{
        debug(CRITICAL, "fromRep2plitProgram: ", lit_rep, " not recognized");
        assert(false);
        setExitCode(SETTINGS::ERROR_CODE);
    }
    clingo_literal_t plit = std::stoi(lit_rep);
    return negative ? not_(plit) : plit ; 
}


std::string getName(const std::unordered_map<clingo_symbol_t, clingo_atom_t>* atomNames, clingo_literal_t plit) {
    std::string prefix = "";

    if (plit == SETTINGS::NONE) { // Assuming 0 represents None in this context
        return SETTINGS::NONE_STR;
    }

    if (plit < 0) {
        prefix = "not ";
    }

    for (const auto& [name, atom] : *atomNames) {
        if (atom == std::abs(plit)) {
            return prefix + from_symbol_to_string(name);
        }
    }

    std::string name = "aux_" + std::to_string(std::abs(plit)) ;
    if(plit < 0) name = negateLiteral(name) ;
    return name;
}

std::string atomNames_to_string(const std::unordered_map<clingo_symbol_t, clingo_literal_t>* atomNames){
    std::ostringstream oss;
    oss<< "{" ;
    for (const auto& [key, value] : (*atomNames)) {
        oss <<"'"<< from_symbol_to_string(key) << "':'" << value<<"', " ;
    }
    oss << "}" ;
    return oss.str() ;
}