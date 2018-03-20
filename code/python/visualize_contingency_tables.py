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

import argparse
import json
import sys

import numpy as np

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

  parser = argparse.ArgumentParser(description="Contingency Table Visualization")

  parser.add_argument("input",
    metavar = "INPUT",
    help    = "Input file"
  )

  parser.add_argument("-f", "--flip",
    required = False,
    action   = "store_true",
    help     = "If set, flips values in the visualization to ensure that quadrant 3 is not used"
  )

  parser.add_argument("-p", "--prune",
    required = False,
    action   = "store_true",
    help     = "If set, prunes duplicates points"
  )

  arguments  = parser.parse_args()
  input_file = arguments.input
  flip       = arguments.flip
  prune      = arguments.prune

  with open(input_file) as f:
    data = json.load(f)

  shapelets = data["shapelets"]
  tables    = []

  for shapelet in shapelets:
    tables.append( shapelet["table"] )

  points = []
  for table in tables:
    x,y = transform_table(table)

    if flip and ( (x < 0 and y < 0) or (np.sign(x) != np.sign(y) and -x > y) ):
      x,y = -y,-x

    points.append( (x,y) )

  if prune:
    points = set(points)

  for x,y in points:
    print("{}\t{}".format(x,y))
