#!/usr/bin/env python3
#
# postprocess_shapelets.py: Analysis the results of a significant
# shapelet mining run.
#
# Input:  JSON file with shapelets
# Output: Unspecified

import json
import math
import sys

import numpy    as np
import networkx as nx

def overlaps(i1,s1,l1,i2,s2,l2):
  """
  Checks whether two shapelets overlap (with respect to the time series
  they originated from. In order to overlap, shapelets need to share at
  least one index and originate from the same time series.
  """

  if i1 != i2:
    return False

  a = s1
  b = s1 + l1 - 1
  c = s2
  d = s2 + l2 - 1

  return a <= d and c <= b
 
if __name__ == "__main__":
  with open(sys.argv[1]) as f:
    data = json.load(f)

  shapelets = data["shapelets"]
  n         = len(shapelets)
  intervals = []
  p_values  = []

  for shapelet in shapelets:
    p = shapelet["p_val"]
    i = shapelet["index"]
    s = shapelet["start"]
    l = len(shapelet["shapelet"])

    intervals.append( (i,s,l) )
    p_values.append(p)

  G = nx.Graph()
  G.add_nodes_from(range(n))

  for i in range(n):
    for j in range(i+1,n):
      if overlaps(*intervals[i], *intervals[j]):
        G.add_edge(i,j)

  components = list(nx.connected_components(G))
  for component in components:
    min_index = -1
    min_p     = np.Inf

    for index in component:
      if p_values[index] < min_p:
        min_index = index
        min_p     = p_values[index]

    print("{0:4d}: {1:0.8e} {2}".format(min_index, min_p, shapelets[min_index]["shapelet"]))
