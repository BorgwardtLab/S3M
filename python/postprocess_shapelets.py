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

  for shapelet in shapelets:
    i = shapelet["index"]
    s = shapelet["start"]
    l = len(shapelet["shapelet"])

    intervals.append( (i,s,l) )

  for i in range(n):
    for j in range(i+1,n):
      if overlaps(*intervals[i], *intervals[j]):
        print(*intervals[i], *intervals[j])
