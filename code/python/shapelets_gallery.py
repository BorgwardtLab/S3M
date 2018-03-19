#!/usr/bin/env python3
#
# shapelets_gallery.py: Given a data set and a shapelet, ranks shapelets
# according to their distance to the input shapelet. This results in one
# gallery that describes how a shapelet relates to *all* time series.
#
# Input:
#   - CSV file with time series
#   - Shapelet
#
# Output: JSON with rank of shapelets (i.e. time series), taking into
# account the class label. 
#
# The output will be written to `stdout`.

import argparse
import csv
import sys

import numpy as np

from scipy.spatial.distance import sqeuclidean

def distance(S,T):
  """
  Calculates the Euclidean distance between two sequences of varying
  lengths, using early abandon if possible.
  
  Returns a tuple comprising the smallest distance, and the sequence
  that minimizes the distance.
  """
  n, m = len(S), len(T)

  if n > m:
    n,m = m,n
    S,T = T,S

  min_distance = np.inf
  min_index    = 0

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
      min_index    = i

  return min_distance, T[min_index:min_index+len(S)]

def format_data(shapelet, offset=0.0):
  """
  Prints a shapelet in a formatted way so that it can be visualized with
  other tools afterwards. The formatted shapelet is returned as a string
  by this function.
  """

  data = ""
  for x,y in enumerate(shapelet):
    data += "{}\t{}\n".format(x+offset,y)

  data += "\n"
  return data

if __name__ == "__main__":
  
  time_series = []
  labels      = []

  parser = argparse.ArgumentParser(description="Shapelet Gallery Creation")

  parser.add_argument("input",
    metavar  = "INPUT",
    help     = "Input file")

  parser.add_argument("-s", "--shapelet",
    required = True,
    nargs    = "+",
    help     = "Shapelet values")

  arguments  = parser.parse_args()
  input_file = arguments.input
  shapelet   = arguments.shapelet

  with open(input_file) as f:
    reader = csv.reader(f)
    for row in reader:

      # Skip comments; some versions of our data files make use of them
      # in order maintain provenance information.
      if row[0].startswith("#"):
        continue

      time_series.append( row[1:] )
      labels.append( float(row[0]) )

  shapelet = [ float(x) for x in shapelet ]
  n        = len(time_series)

  data_cases    = []
  data_controls = []

  for index, series in enumerate(time_series):
    d, s = distance(shapelet, series)

    if labels[index] == 0.0:
      data_cases.append( (d,s) )
    else:
      data_controls.append( (d,s) )

  assert len(data_cases) + len(data_controls) == n

  data_cases    = sorted(data_cases,    key=lambda x: x[0])
  data_controls = sorted(data_controls, key=lambda x: x[0])

  for data_case, data_control in zip(data_cases, data_controls):
    _, case    = data_case
    _, control = data_control

    sys.stdout.write(format_data(case))
    sys.stdout.write(format_data(control, offset=2*len(shapelet)))
    sys.stdout.write("\n\n")
