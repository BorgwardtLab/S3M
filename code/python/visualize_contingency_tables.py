#!/usr/bin/env python3
#
# visualize_contingency_tables.py: Visualizes all contingency tables
# obtained by our method in the form of a diagram in the plane.
#
# Input:  JSON file with shapelets
#
# Output: A set of points in the plane, each representing one table,
#         such that the distance to the origin refers to preferences
#         in splitting behaviour.
#
# The output will be written to `stdout`.

import json
import math
import sys

import numpy as np

# TODO: make configurable
flip = True

def transform_table(table):
  """
  Transforms a contingency table into a point on a two-dimensional
  plane, in which the distance to the origin shows the suitability
  of a contingency table for separating cases and controls.
  """
  
  # Yes, this ordering is correct. Please refer to our paper for
  # more details.
  a, b, d, c = table

  n1 = a+b
  n0 = c+d

  return (a-b) / n1, (c-d) / n0

if __name__ == "__main__":
  with open(sys.argv[1]) as f:
    data = json.load(f)

  shapelets = data["shapelets"]
  tables    = []

  for shapelet in shapelets:
    tables.append( shapelet["table"] )

  for table in tables:
    x,y = transform_table(table)

    if flip and ( (x < 0 and y < 0) or (np.sign(x) != np.sign(y) and -x > y) ):
      x,y = -y,-x

    print("{}\t{}".format(x,y))
