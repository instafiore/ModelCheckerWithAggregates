import sys
import re
from typing import Dict, List
import clingo
import argparse
from prop_unf.utils.settings import *
import pandas as pd

def debug(*message: str, end ="\n", file = sys.stderr):
    print(message, end=end, file=file)
    sys.stderr.flush()
        

def removeCommas(string: str) -> str:
    string =  re.sub(r"^,","",string)
    string =  re.sub(r",+$","",string)
    return string

def parse_args(checkCorrectness = False):
    nModelsDefault = 0 if checkCorrectness else 1
    timeLimit = TIMELIMIT if checkCorrectness else 1 * 60 * 60 * 24
    parser = argparse.ArgumentParser(description='Propagator for recursive aggregates' if not checkCorrectness else 'Checker for Recursive Aggregates Propagator')
    parser.add_argument("-e", "--encoding", required=not checkCorrectness, help="Path to encoding file")
    parser.add_argument("-i", "--instance", help="Path to instance file (optional)")
    parser.add_argument("-l", "--logfile", help="Define log file", default=f"{LOGS}/logfile")
    parser.add_argument("-s", "--stats", choices=[1, 2, 3], type=int, help="Define stats mode", default=0)
    parser.add_argument("-b", "--build_mode", choices=["test","release"], type=str, help="Define build mode")
    parser.add_argument("-n", "--num_models", default = nModelsDefault, type=int, help="Define number of models")
    parser.add_argument("-m", "--method", choices=["reduct", "unfounded", "aspreduct"] , default = "reduct", type=str, help="Define method to detect unfounded set, default is reduct")
    parser.add_argument("-enq", "--enqueue", choices=["True","False"] , default = "True", type=str, help="Whether to enqueue not propagated unfounded atoms")
    parser.add_argument("-pc", "--partial_check", choices=["True","False"] , default = "True", type=str, help="Whether to enqueue not propagated unfounded atoms")
    parser.add_argument("-tr_pc", "--treshold_rate_partial_check", default = 0.025, type=float, help="Minimum rate change percentage to activate partial checks (default: 0.025)")
    parser.add_argument("-ti_pc", "--treshold_internal_partial_check", default = 0.20, type=float, help="Minimum percentage of true internal atoms to activate partial checks (default: 0.20)")

    if checkCorrectness:
        subparsers = parser.add_subparsers(dest="check", required=True, help="Check mode: all -> check all answer sets, unsat -> check unsat in exp file, instance -> check computed answer set")
      
        parser_all = subparsers.add_parser("all", help="Checking all answer sets")

        parser_unsat = subparsers.add_parser("unsat", help="Checking unsat in exp file")
        parser_unsat.add_argument("-uf", "--unsatfile", required=True, help="Path to exp file experiments")

        parser_unsat_clown = subparsers.add_parser("unsat-clown", help="Checking unsat in exp file")
        parser_unsat_clown.add_argument("-uf", "--unsatfile", required=True, help="Path to exp file experiments")
  
        parser_instance = subparsers.add_parser("instance", help="Checker for computed answer set")
        parser_instance.add_argument("-f", "--file_answerset", required=True, help="Path to output file containing answer set")
      
    else:
        parser.add_argument("-c", "--check", action="store_true", help="Check answer set")


    dict_res = vars(parser.parse_args())
   
    return dict_res

def convertoIntoAssumptionsCPP(answerset: List[str]) -> List[str]:
    
    # Print the assumptions in C++ format
    for atom in answerset:
        if atom.startswith("~"):
            p = atom[1:]
            print(f'assumptions.push_back(-(*program.rep2plit)["{p}"]);')
        else:
            print(f'assumptions.push_back((*program.rep2plit)["{atom}"]);')


def getAnswerSetsFromFile(file_path: str) -> List[List[str]]:
    answersets = []
    with open(file=file_path, mode="r") as file:
        checkFile = False
        for line in file:
            matchSat = re.search(REGEX_ANSWERSET,line)
            matchUnsat = re.search(r"^UNSAT$",line)
            if matchSat:
                answerset = matchSat.group(1).split('\t')
                # print(f"answerset file: {answerset}")
                answersets.append(answerset)
                checkFile = True
            elif matchUnsat:
                answersets = None
                checkFile = True
                break
    if not checkFile:
        print(f"File {file_path} is empty or does not contain any answer set.")
        exit(1)
    return answersets


def check(encoding_path: str, instance_path: str, file_path: str) -> bool:
    answersets = getAnswerSetsFromFile(file_path)
    success = True
    sat = not answersets is None
    if sat:
        for answerset in answersets:
            resCheck = checkAnswerSet(encoding_path, instance_path, answerset)
            if not resCheck:
                debug(f"[SAT] resCheck: {resCheck} for enc: {encoding_path} ins: {instance_path} answerset: {answerset}")
                success = False
    else:
        return True
        success = checkUnsat(encoding_path, instance_path)
        debug(f"[UNSAT] resCheck: {success} for end: {encoding_path} ins: {instance_path}")
    return success

def checkUnsatFromResultFile(file_path_csv: str) -> bool:
    df = pd.read_csv(file_path_csv, header='infer', sep=',')
    base = "clingo_plain"
    df = df[(df["EXIT_CODE"] == 20) | (df["EXIT_CODE"] == 137)].groupby(["EXECUTABLE"])

    df_base = df.get_group(base)
    unsat_timeout_base = set(df_base["INSTANCE"].values)
    print(f"base: {base}")

    for exe in df.groups:
        if exe == base:
            continue
        df_group = df.get_group((exe))
        df_group = df_group[df_group["EXIT_CODE"] == 20]
        print("Checking executable: ", exe)
        unsat = set(df_group["INSTANCE"].values)
        n = len(unsat)
        diff = unsat.difference(unsat_timeout_base)
        if(len(diff) > 0):
            print(f"Found {n} unsat instances for {exe} that are not in the base: {unsat}")
            print(f"They are: {diff}")
            return False
            

    return True

def check_unsatisfability_clown(file):
    df = pd.read_csv(file, delimiter='\t', names=["problem", "instance", "executable", "method", "enq", "pc", "full_instance", "status_1", "status_2", "status", "exit_code", "real", "time", "user", "system", "memory"])
    
    satCodes = [10, 30]
    print(f"checking correctness for unsatisfability...")
    df_unsat = df[(df["method"] != "plain") & (df["exit_code"] ==  20)] 
    
    df_plain_sat = df[(df["method"] == "plain") & (df["exit_code"].isin(satCodes))]

    amosum_instances = set(df_unsat["instance"])
    plain_instances = set(df_plain_sat["instance"])
    
    common_instances = amosum_instances.intersection(plain_instances)

    if common_instances:
        print(f"Error: Some instances appear in both df_amosum_unsat and df_plain_sat!")
        print(common_instances) 
        return False 
    
    return True  


def checkUnsat(encoding_path: str, instance_path: str):
    return checkAnswerSet(encoding_path, instance_path, None)

def checkAnswerSet(encoding_path: str, instance_path: str, answerset: List[str]) -> bool:
    ctl = clingo.Control()
    
    # Load encoding file
    ctl.load(encoding_path)
    
    # Load instance file if provided
    if instance_path:
        ctl.load(instance_path)
    
    # Generate constraints for the given model
    sat = not answerset is None
    if sat:
        constraints = []
        for atom in answerset:
            if atom.startswith("~"):
                constraints.append(f":- {atom[1:]}.")  # Negative atom constraint
            else:
                constraints.append(f":- not {atom}.")  # Positive atom constraint
        
        # Add constraints to the program
        checker_file = "\n".join(constraints)
        debug(f"checker_file: \n{checker_file}")
        ctl.add("base", [], checker_file)
    
    # Ground and solve
    ctl.ground([("base", [])])
    with ctl.solve(yield_=True) as handle:
        for _ in handle:
            return sat  # SAT
    return not sat  # UNSAT



def run_clingo(args: Dict):
    """
    Runs clingo with the given encoding and instance to enumerate models.

    Parameters:
    encoding (str): Path to the encoding file.
    instance (str): Path to the instance file [not mandatory].
    n (int): Number of models to enumerate (0 for all models). Default is 1.

    Returns:
    list of sets of strings: Each set represents an answer set.
    or
    str: 'UNSAT' if the program is unsatisfiable.
    """
    ctl = clingo.Control()

    # Load the encoding file
    ctl.load(args["encoding"])

    # If an instance is provided, add it as a fact
    if args["instance"]:
        ctl.load(args["instance"])

    ctl.ground([("base", [])])

    # Set the number of models to enumerate
    ctl.configuration.solve.models = args["num_models"]

    answer_sets = []

    def on_model(model):
        answer_sets.append(set(str(atom) for atom in model.symbols(atoms=True)))

    result = ctl.solve(on_model=on_model)

    # If no answer sets were found, return 'UNSAT'
    if not answer_sets:
        return "UNSAT"

    return answer_sets

