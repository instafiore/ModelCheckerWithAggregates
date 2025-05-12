#!/usr/bin/env python3
import argparse
import sys
import os


CORRECTNESS = os.path.dirname(os.path.abspath(__file__))
TESTS = os.path.dirname(CORRECTNESS)
ROOT_C = os.path.dirname(os.path.dirname(CORRECTNESS))
sys.path.append(ROOT_C)

from prop_unf.utils.settings import *
from prop_unf.utils.utility import *
sys.path.append(PROP_UNF)
sys.path.append(CORRECTNESS)
sys.path.append(TESTS)
# print(f"sys.path: {sys.path}")
from checker import Checker
from prop_unf.utils.settings import ROOT, LOGS


def main():
    args = parse_args(checkCorrectness=True)
    args["root"] = ROOT
    if args["check"] == "all":
        print(f"[python] parameters: {args}")
        args["num_models"] = 0
        checker = Checker(args)
        checker.check_correctness()
    elif args["check"] == "unsat":
        success = checkUnsatFromResultFile(args["unsatfile"])
        s = "Passed " if success else "Failed"
        print(f"{s} check unsat for file: {args['unsatfile']}")
        if success:
            exit(0)
        else:
            exit(1)
    elif args["check"] == "unsat-clown":
        success = check_unsatisfability_clown(args["unsatfile"])
        s = "Passed " if success else "Failed"
        print(f"{s} check unsat for file: {args['unsatfile']}")
        if success:
            exit(0)
        else:
            exit(1)
    else:
        success = check(args["encoding"],args["instance"],args["file_answerset"])
        s = "Passed " if success else "Failed"
        e = args["encoding"]
        i = args["instance"]
        f = args["file_answerset"]
        print(f"{s} check correctness for encoding: {e} instance: {i} file: {f}")
        if success:
            exit(0)
        else:
            exit(1)

            
if __name__ == "__main__":
    main()
