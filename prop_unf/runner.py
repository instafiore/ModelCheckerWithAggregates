import re
import subprocess
from typing import List, Set
from utils.settings import *
from utils.utility import *
import sys

class Runner:
    def __init__(self, args):
        self.args = args
        self.exit_code = None
        self.printOutput = True

    def parseAnswerSets(self,lines_output: List[Set[str]]) ->  List[Set[str]]:
        answer_sets = []
        # answer_sets_with_negative = []
        for line in lines_output:
            if not re.search(REGEX_ANSWERSET, line) is None:
                answer_set_str = re.search(REGEX_ANSWERSET, line).group(1)
                # print(f"find all: {re.findall(GENERIC_REGEX_ANSWERS_SET_ATOM, answer_set_str)}")
                # answerSetList = [removeCommas(match[0]) for match in re.findall(GENERIC_REGEX_ANSWERS_SET_ATOM, answer_set_str)]
                answerSetList = answer_set_str.split("\t")
                # answer_set = set([a for a in answerSetFindAll if not a.startswith("~")])
                answer_set = set([a for a in answerSetList if not a.startswith("~")])
                # answer_set_with_negative = set(answerSetFindAll)
                answer_sets.append(set(answer_set))
                # answer_sets_with_negative.append(set(answer_set_with_negative))
        # return answer_sets, answer_sets_with_negative
        return answer_sets

    # def printAnswerSetsForChecking(self, answerSetsWithNegatives):
    #     for answerSet in answerSetsWithNegatives:
    #         print(f"answerSet: {answerSet}")

    def run(self) -> List[Set[str]]:

        if self.args["build_mode"]:
            build_mode = self.args["build_mode"]  
            cmd = [f"make -C {PROPAGATOR_C} {build_mode} -j"]
            cmd = ' '.join(cmd)
            print(f"[python] {cmd}")
            subprocess.run(cmd, check=True, shell=True)     

        # Run the propagator command
        cmd = [PROPAGATOR_BIN] + [f"-{key}={value}" for key, value in self.args.items()]
        cmd = ' '.join(cmd)
        print(f"[python] command: {cmd}")
        output_error = subprocess.run(cmd, capture_output=True, text=True, shell=True)
        self.exit_code = output_error.returncode
        error = output_error.stderr.strip()
        output = output_error.stdout.strip()
        if(error != ""): print(error, file=sys.stderr)
        if self.printOutput: print(f"{output}")
       
        # answerSets, answerSetsWithNegatives = self.parseAnswerSets(output.splitlines())
        answerSets = self.parseAnswerSets(output.splitlines())
        # print(f"[python] answerSets: {answerSets}")
        # print(f"[python] exit code: {self.exit_code}")
        # self.printAnswerSetsForChecking(answerSetsWithNegatives)
        return answerSets
