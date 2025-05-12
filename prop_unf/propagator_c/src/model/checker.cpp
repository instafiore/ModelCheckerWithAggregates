#include "checker.h"
#include "../builder/encoding_builder.h"
#include "../utils/utility.h"

Checker::Checker(std::string encoding_path, std::string instance_path, std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, std::unordered_map<std::string, std::string>& params)
    :encoding_path(encoding_path),instance_path(instance_path), atomNames(atomNames), params(params){

}

void Checker::init(std::string encoding_path, std::string instance_path,std::unordered_map<clingo_symbol_t, clingo_atom_t>& atomNames, std::unordered_map<std::string, std::string>& params){
    if(instance == nullptr){
        instance = new Checker(encoding_path, instance_path, atomNames, params);
    }
}

Checker* Checker::getInstance(){
    if(instance == nullptr) 
    return instance;
}

bool Checker::__check__(std::vector<clingo_symbol_t>* model, bool sat){
    // clingo_control_t *c = nullptr;  // Ensure it's properly initialized
    // ctl = nullptr;  // Ens
    // clingo_control_t *ctl = nullptr;  
    // std::cout << "Before clingo_control_new: c = " << c << std::endl;
    // std::cout << "Before clingo_control_new: ctl = " << ctl << std::endl;

    // Create control with no arguments
    handle_error(clingo_control_new(nullptr, 0, nullptr, nullptr, 0, &ctl));

    // std::cout << "After clingo_control_new: c = " << c << std::endl;
    // std::cout << "After clingo_control_new: ctl = " << ctl << std::endl;
    // exit(0);
    clingo_part_t parts[] = {{"base", NULL, 0 }};
    
    debug(INFO, "[CHECKER] Loading encoding: ", encoding_path, " for checking");
    handle_error(clingo_control_load(ctl, encoding_path.c_str())) ;
    if (instance_path != SETTINGS::NONE_STR){
        debug(INFO, "[CHECKER] Loading instance: ", instance_path, " for checking");
        handle_error(clingo_control_load(ctl, instance_path.c_str()));
    }
    
    if(sat){
        std::string checker_program = "" ;
        for(const auto&[sym, plit] : atomNames){
            std::string atomName = from_symbol_to_string(sym);
            if(std::find(model->begin(), model->end(), sym) != model->end()){
                // false literal
                atomName = negateLiteral(atomName);
            }
        std::string cons = ":- "+atomName+".";
        checker_program += cons + "\n";
        }
        // debug(INFO, "[CHECKER] Checker program:\n", checker_program);
        handle_error(clingo_control_add(ctl, "base", NULL, 0, checker_program.c_str()));
    }
    

    debug(INFO, "[CHECKER] Starting grounding checker");
    handle_error((clingo_control_ground(ctl, parts, 1, NULL, NULL)));

    std::vector<clingo_literal_t> assumptions ;
    std::vector<clingo_atom_t> answersetChecker ;
    int exitCode = solve("checker", ctl, assumptions);
    clingo_control_free(ctl);
    ctl = nullptr ;
    return (!sat || exitCode == SETTINGS::SAT_CODE) && (sat || exitCode == SETTINGS::UNSAT_CODE) ;
}

bool Checker::checkSat(std::vector<clingo_symbol_t>& model){
    return __check__(&model, true);
}

bool Checker::checkUnsat(){
    return __check__(nullptr, false);
}

void Checker::cleanup(){
    if(instance != nullptr) delete instance ;
}

Checker::~Checker(){
    if (ctl == NULL) { clingo_control_free(ctl); }
}

inline Checker* Checker::instance = nullptr;