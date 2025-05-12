#!/usr/bin/env python

import os

id = 201
for i in range(10, 101, 20):
    for j in [.2,.4,.6,.8,1]:
        for k in range(1,5):
            os.system(f"./generate.py {i} {j} > {id}-graph_colouring-{i}-0.new.asp")
            id+=1