#!/usr/bin/env python

import networkx as nx
import sys

def generate(v, p):

    graph = nx.erdos_renyi_graph(v, p)

    for node in graph.nodes():
        print(f"node({node}).")

    for edge in graph.edges():
         print(f"link({edge[0]},{edge[1]}).")

    print("colour(red0).\
    colour(green0).\
    colour(blue0).\
    colour(yellow0).\
    colour(cyan0).")

if __name__ == "__main__":
    vertices = int(sys.argv[1])
    p = float(sys.argv[2])
    generate(vertices, p)