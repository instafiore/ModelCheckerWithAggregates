#!/usr/bin/env python
import argparse
import sys
import os

PROP_UNF = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(PROP_UNF)
sys.path.append(ROOT)
sys.path.append(PROP_UNF)

from utils.settings import *
from utils.utility import *
sys.path.append(ROOT)
sys.path.append(PROP_UNF)

from runner import Runner

def main():
  
    args = parse_args(checkCorrectness=False)    
    args["root"] = ROOT
    # print(f"sys.path: {sys.path}")
    print(f"[python] parameters: {args}")

    runner = Runner(args)  # Pass args as a dictionary
    runner.run()

    exit(runner.exit_code)

if __name__ == "__main__":
    main()
