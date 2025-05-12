import re
import subprocess
from typing import Dict, List, Set
from prop_unf.utils.settings import *
from prop_unf.utils.utility import *
from prop_unf.runner import *
import sys

class Checker:
    def __init__(self, args: Dict):
        self.runner = Runner(args)
        self.args = args

    def check_correctness(self) -> bool:
        """
        Checks if each answer set produced by the propagator is also produced by clingo.

        Returns:
        bool: True if all propagator's answer sets are found in clingo's answer sets, False otherwise.
        """
        # Run the propagator
        self.runner.printOutput = False
        propagator_answer_sets = self.runner.run()
        
        if propagator_answer_sets == "UNSAT":
            debug("Propagator found UNSAT")
            propagator_answer_sets = []


        # Run clingo
        clingo_answer_sets = run_clingo(self.args)
        if clingo_answer_sets == "UNSAT":
            debug("Clingo found UNSAT")
            clingo_answer_sets = []

        correct = True 
        # Compare answer sets
        for pas in propagator_answer_sets:
            if pas not in clingo_answer_sets:
                debug(f" => Answer set {pas} from propagator not found in clingo's answer sets.")
                correct = False
                break
        
        for pas in clingo_answer_sets:
            if pas not in propagator_answer_sets:
                debug(f" <= Answer set {pas} from clingo not found in propagator's answer sets.")
                correct = False
                break

        if not correct:
            # debug("propagator_answer_sets: ")
            # for ans in propagator_answer_sets:
            #     debug(ans)
            # debug("clingo_answer_sets: ")
            # for ans in clingo_answer_sets:
            #     debug(ans)
            exit(1)

        debug("All propagator's answer sets are valid.")
        exit(0)