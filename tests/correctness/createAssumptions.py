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
    answer_set = """
    neg(1)    neg(2)  ~neg(3) ~id_3(0)        id_3(4) ~id_3(3)        id_3(2) neg(4)  edge(2,4)       vertex(3)      id_2(0)  edge(1,3)       ~id_2(4)        id_2(3) ~id_2(2)        edge(1,0)       ~id_2(1)        ~pos(4) saturate        id_1(3)~id_1(4) ~id_1(2)        ~id_1(1)        s(3)    ~s(4)   ~s(2)   x(0)    edge(3,0)       vertex(1)       x(2)    edge(0,1)      edge(2,1)        ~s(1)   ~pos(1) ~x(1)   edge(4,1)       s(0)    edge(4,2)       id_3(1) id_1(0) vertex(4)       pos(0)  edge(0,3)       vertex(0)       edge(1,2)       pos(3)  ~neg(0) vertex(2)       ~x(4)   pos(2)  edge(2,0)       edge(0,2)       x(3)   edge(3,1)        edge(1,4)
    """
    convertoIntoAssumptionsCPP(answer_set.split())

            
if __name__ == "__main__":
    main()
