#!/usr/bin/env python3
import argparse
import re
import os
import sys
import random
from typing import Dict, List, Tuple

import numpy as np

random.seed(42)

class InstanceBuilder:
    
    suffix = "new1"

    def __init__(self, e):
        self.e = e
        self.a = None
        self.k = None

    def build(self):
        self._prepare_()
        self.instance.extend([f"var(exists,x{x+1},{self.exists[x]})." for x in range(len(self.exists))])
        self.instance.extend([f"var(all,y{y+1},{self.forall[y]})." for y in range(len(self.forall))])
        self.instance.append(f"k({self.k}).")
        return self.instance

    def _prepare_(self):
        self.instance = []
        self._create_exists()
        self._create_k()
        self._create_forall()

    def __repr__(self):
        return f"middle_{InstanceBuilder.suffix}"
    
    def __str__(self):
        return self.__repr__()
    
    def _create_exists(self):
        self.exists = []
        for _ in range(self.e):
            self.exists.append(random.randint(1, self.e))    
    
    def _create_forall(self):
        self.a = self.e
        self.forall = []
        for _ in range(self.a):
            self.forall.append(random.randint(1, self.a))    

    def _create_k(self):
        '''
        reasoning: 
            the expected value is e - 1 / 2 -> e - 1 / 2 * e is the expected value for the max possible sum for exists.
            Given that the same applies for the foralls then the total mps is e - 1 / 2 * e * 2 = e - 1 * e = e^2 - e
        '''
        mps = self.e**2 - self.e
        self.k = random.randint(1, mps)

class SatInstanceBuilder(InstanceBuilder):    

    def _create_forall(self):
        assert self.k is not None
        self.a = self.e
        self.forall = []
        for _ in range(self.a):
            self.forall.append(random.randint(self.k + 1, self.k + self.a))    

    def _create_k(self):
        assert self.exists is not None
        self.k = int(random.choice(self.exists)) -1
    
    def __repr__(self):
        return f"sat_{InstanceBuilder.suffix}"
    
class PUnsatInstanceBuilder(InstanceBuilder):    

    def _prepare_(self):
        self.instance = []
        self._create_k()
        self._create_exists_forall()
    
    def _create_k(self):
        self.k = int(random.choice(range(1,self.e+1)))

    def _create_exists_forall(self):
        self.exists = []
        self.forall = []
        for _ in range(self.e):
            exist = random.randint(1, self.e)
            self.exists.append(exist) 
            n = self.k - exist 
            div = random.randint(3,5)
            quo = n // div 
            change = n % div
            for _ in range(div):
                if quo != 0 and quo not in self.forall: self.forall.append(quo)
            if change != 0 and change not in self.forall: self.forall.append(change)
        self.a = len(self.forall)



    
    
    def __repr__(self):
        return f"punsat_{InstanceBuilder.suffix}"

    
class BenchmarkCreator():

    instances_type_distribution: Dict[str, Tuple[InstanceBuilder, float]] = {
        "sat": (SatInstanceBuilder, 0.33),
        "middle": (InstanceBuilder, 0.33),
        "punsat": (PUnsatInstanceBuilder, 0.34),
    }

    def __init__(self, args):
        self.args = args
        self.instances = []

    def print_instance(self, builder: InstanceBuilder):
        common_lines = []
        instance = builder.build()
        self.instances.extend(instance)
        assert builder.a is not None
        assert builder.k is not None
        file_name = f"gss-{builder.e}-{builder.a}-{builder.k}_{builder}.asp"
        with open(f"{file_name}", "w") as file:
            file.write("\n".join(common_lines))
            file.write("\n")
            file.write("\n".join(instance))
            
            

    def create_benchmark(self):
        self.instances = []
        starting_e = 10
        for i in range(self.args["num_instances"]):
            p = random.random()
            builder: InstanceBuilder = None
            for builder_name in BenchmarkCreator.instances_type_distribution:
                f = BenchmarkCreator.instances_type_distribution[builder_name][0]
                p_f = BenchmarkCreator.instances_type_distribution[builder_name][1]
                if(p <= p_f):
                    builder = f(starting_e)
                    break
                p -= p_f
            starting_e += 10
            self.print_instance(builder)

def main(argv):
    
    parser = argparse.ArgumentParser(description='Creator of Generalized Subset Sum Domain')
    parser.add_argument("-n", "--num_instances", required=True, type=int , help="Number of instances")
    args = vars(parser.parse_args())


    benchmarkCreator = BenchmarkCreator(args)   
    # benchmarkCreator.create_benchmark()

    benchmarkCreator.print_instance(InstanceBuilder(11))
 
if __name__ == "__main__":
    main(sys.argv)