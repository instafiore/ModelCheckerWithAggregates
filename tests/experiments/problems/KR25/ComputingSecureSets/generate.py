#!/usr/bin/env python

import networkx as nx
import sys

def generate(v, p):

    graph = nx.erdos_renyi_graph(v, p)

    for edge in graph.edges():
         print(f"edge({edge[0]},{edge[1]}).")

if __name__ == "__main__":
    vertices = int(sys.argv[1])
    p = float(sys.argv[2])
    generate(vertices, p)