#!/usr/bin/env python3
#
# calculate_shapelet_distances.py: Performs basic distance (dissimilarity)
# analysis of a set of shapelets. This permits post-processing the output
# of S3M, the main executable of this project.
#
# Input:  JSON file with shapelets
# Output: Matrix of pairwise distances of shapelets
#
# The output will be written to `stdout`.

import json
import sys

import numpy as np

from scipy.spatial.distance import sqeuclidean

def distance(S,T):
  """
  Calculates the Euclidean distance between two sequences of varying
  lengths, using early abandon if possible.
  """
  n, m = len(S), len(T)

  if n > m:
    n,m = m,n
    S,T = T,S

  min_distance = np.inf

  for i in range(0, m - n + 1):
    stop         = False
    sum_distance = 0.0

    for j,x in enumerate(S):
      y             = T[i+j]
      sum_distance += sqeuclidean(x, y)

      # Abandon calculations as soon as the best distance (so far) has
      # been surpassed---adding more values will only increase it.
      if sum_distance >= min_distance:
        stop = True
        break

    if not stop:
      min_distance = sum_distance

  return min_distance

if __name__ == "__main__":
  with open(sys.argv[1]) as f:
    data = json.load(f)

  shapelets = data["shapelets"]
  n         = len(shapelets)
  distances = np.zeros(shape=(n,n), dtype="float32")

  for i in range(n):
    for j in range(i+1,n):
      distances[i,j]                          \
        = distance(shapelets[i]["shapelet"],  \
                   shapelets[j]["shapelet"])

      distances[j,i] = distances[i,j]

  np.savetxt(sys.stdout, distances, fmt="%.8f")
