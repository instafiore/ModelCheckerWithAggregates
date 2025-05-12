#!/usr/bin/env python

import os

# for i in range(20,201,20):
#     for j in [.2,.4,.6,.8,1]:
#         for k in range(1,6):
#             os.system("generate.py %d %f > g-%d-%d-%d.new.asp" % (i,j,i,j*100,k))

# for i in range(10, 101, 20):
#     for j in [.2,.4,.6,.8,1]:
#         for k in range(1,3):
#             os.system(f"./generate.py {i} {j} > g-{i}-{j*100}-{k}.new.asp")

n = 13
p = 0.6
k = 1
os.system(f"./generate.py {n} {p} > correctness.g-{n}-{p*100}-{k}.asp")
