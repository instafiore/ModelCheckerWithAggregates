#include <iostream>
#include "propagator/propagator_abstraction.h"
#include <clingo.h>
#include <string>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <limits>
#include <climits>
#include <chrono>
#include "utils/utility.h"
#include "settings.h"
#include "utils/logger.h"
#include "model/model.h"
#include "model/checker.h"
#include "propagator/propagator_initializer.h"
#include <csignal>


void signalHandler(int signum) {
    print("Received SIGINT (Ctrl+C)");
    #ifdef LOGGER_ACTIVE
    clingo_control* ctl =  PropagatorInitializer::getInstance()->program.ctl;
    if(ctl == nullptr) return ;
    const clingo_statistics_t *stats;
    uint64_t stats_key;
    Logger::getInstance()->logf(SIGINTLOGGER,"Received SIGINT (Ctrl+C)");
    // prinStructStats(true);
    prinStats(true);
    if(!clingo_control_statistics(ctl, &stats)){goto error;}
    if(!clingo_statistics_root(stats, &stats_key)){goto error;}
    if(!print_clingo_statistics(stats, stats_key, 0)){goto error;}
    Logger::getInstance()->log(STATS, "Stats Clingo:");
    printClingoStats(stats, stats_key, 1);
    #endif
error:
    exit(signum);
}

bool init(clingo_propagate_init_t *_init, PropagatorAbstraction* propagator){return propagator->init(_init);}
bool propagate(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size, PropagatorAbstraction* propagator){ return propagator->propagate(control, changes, size);}
void undo(clingo_propagate_control_t *control, const clingo_literal_t *changes, size_t size, PropagatorAbstraction* propagator){ propagator->undo(control, changes, size);}

clingo_ground_program_observer_t observer = {
    .rule = rule_callback,
    .weight_rule = weight_rule_callback,
};

int main(int argc, char const *argv[]) {

    signal(SIGINTLOGGER, signalHandler);
    
    std::unordered_map<std::string, std::string> params =  init_param(argc, argv);
    SETTINGS::ROOT = params.find("root")->second;
    bool lazy_logger = false;


    HandleStats::getInstance()->add_stat(new Stat("full_check_time", true));
    HandleStats::getInstance()->add_stat(new Stat("partial_check_time", true));
    HandleStats::getInstance()->add_stat(new Stat("reason_time", true));
    HandleStats::getInstance()->add_stat(new Stat("reason_size", false));
    HandleStats::getInstance()->add_stat(new Stat("propagate_time", true));


    
    const std::filesystem::path log_file = params.find(SETTINGS::LOGFILE)->second;
    
    initLogger(log_file, lazy_logger);


    // debug(INFO,"Root directory: ", SETTINGS::ROOT);
    debug(INFO,"Root directory: ", SETTINGS::ROOT);
 
    debug(INFO,"Log file: ", log_file);

    int major, minor, revision;
    clingo_version(&major, &minor, &revision);
    debug(INFO, "Clingo version: ", major, ".", minor, ".", revision);

    std::string encoding_path = "" ;
    if(params.find(SETTINGS::ENCODING)->second != SETTINGS::NONE_STR){
        encoding_path = params.find(SETTINGS::ENCODING)->second;
        debugf(DEBUG, "Encoding path: ", encoding_path);
    }else{
        debug(ERROR, "No encoding path provided");
        return 1;
    }

    std::string instance_path = params.find(SETTINGS::INSTANCE)->second ;
    if(instance_path != SETTINGS::NONE_STR){
        debugf(DEBUG,"Instance path: ", instance_path);
    }

    clingo_solve_result_bitset_t solve_ret;
    clingo_control_t *ctl = NULL;
    std::string n0 = params.find(SETTINGS::NMODELS)->second ;
    int sizeArgs = 3 ;
    
    // unsigned time_limit = std::stoi(params.find(SETTINGS::TIMELIMIT)->second);

    int stats_value = std::stoi(params["stats"]) ;
    std::string stats_str =  "--stats=" + std::to_string(stats_value) ;
    const char *args[] = {"--seed=42", "--no-ufs-check", n0.c_str(), stats_str.c_str()};
    if(stats_value > 0){
        sizeArgs+=1 ;
    }

    // const char *args[] = {"--seed=42", "--no-ufs-check", n0.c_str(), "--configuration=trendy"};
    // ++sizeArgs;

    handle_error(clingo_control_new(args, sizeArgs, logger, NULL, UINT_MAX, &ctl));
    clingo_part_t parts[] = {{"base", NULL, 0}};
    std::vector<PropagatorAbstraction*> propagators;

    clingo_propagator_t prop = {
        (clingo_propagator_init_callback_t)init,
        (clingo_propagator_propagate_callback_t)propagate,
        (clingo_propagator_undo_callback_t)undo,
        NULL,
        NULL,
    };
   
    Program &program = PropagatorInitializer::getInstance()->program; 
    handle_error(clingo_control_register_observer(ctl, &observer, false, &program));
    
    handle_error(clingo_control_load(ctl, encoding_path.c_str())) ;
    if (instance_path!= SETTINGS::NONE_STR) handle_error(clingo_control_load(ctl, instance_path.c_str()));
    
    debug(INFO, "Starting grounding");
      
    
    handle_error((clingo_control_ground(ctl, parts, 1, NULL, NULL)));
    

    size_t N = program.max_plit + 1 ; 
    debug(INFO, "program.max_plit: ", program.max_plit);
    InterpretationFunction::getInstance(N);
   
    
    debug(DEBUG, "All program:\n", program.toString());
    bool isNonHFCProgram = false ;
    
    program.computeNonHFCComponents();

    
    for(auto& component: program.getComponents()){
        register_propagator(ctl, prop, component, params, propagators);
        debug(INFO,"Registering propagator with\n",component->toString());
        isNonHFCProgram = true ;
    }

    
    if(program.getComponents().size() == 0){
        debug(INFO, "Program has no no-hfc component");
    }else{
        debug(INFO, "Number of Non-HCF: ",program.getComponents().size());
    }

    debug(INFO, "initProgram running in main");
    program.init(ctl, nullptr, false);
    std::vector<clingo_literal_t> assumptions;
    // assumptions.push_back((*program.rep2plit)["b"]);
    


    Checker::init(encoding_path, instance_path, *program.atomNames, params);

    debug(INFO, vectorToString(assumptions, "assumptions: "));
    debug(INFO, "Starting solving");    

    // Solving step
    handle_error(solveMain(params, program ,ctl, &solve_ret, assumptions.data(), assumptions.size()));

    // exit(0);
    // Cleaning up
    for(auto& propagator: propagators){
        if(propagator != nullptr) delete propagator;
    }

    int res = getErrorCodeFromResult(solve_ret, "program");
    setExitCode(res);
    if(SETTINGS::EXIT_CODE == SETTINGS::UNSAT_CODE){
        print("UNSAT");
        if(params.find("check")->second == SETTINGS::TRUE_STR){
            bool check = Checker::getInstance()->checkUnsat();
            if(check){
                print("Check UNSAT PASSED! V");
            }else{
                print("Check UNSAT FAILED! X");
                debug(ERROR, "Check UNSAT FAILED! X");
                setExitCode(SETTINGS::ERROR_CODE);
            }
        }
    }

    
    // get the statistics object, get the root key, then print the statistics recursively
    #ifdef LOGGER_ACTIVE
    const clingo_statistics_t *stats;
    uint64_t stats_key;
    handle_error(clingo_control_statistics(ctl, &stats));
    handle_error(clingo_statistics_root(stats, &stats_key));
    handle_error(print_clingo_statistics(stats, stats_key, 0));
    Logger::getInstance()->log(STATS, "Stats Clingo:");
    printClingoStats(stats, stats_key, 1);
    // prinStructStats(true);
    prinStats(true);
    #endif
   
    Checker::getInstance()->cleanup();
    cleanupLogger();
    print("EXIT_CODE: ",SETTINGS::EXIT_CODE);
    if (ctl) { clingo_control_free(ctl); }
    if(isNonHFCProgram) PropagatorInitializer::cleanup();
    return SETTINGS::EXIT_CODE;
}