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

  return min_distance, T[i,i+len(S)+1]

if __name__ == "__main__":
  
  time_series = []
  labels      = []

  with open(sys.argv[1]) as f:
    reader = csv.reader(f)
    for row in reader:
      time_series.append( row[1:] )
      labels.append( row[0] )

  n = len(time_series)
