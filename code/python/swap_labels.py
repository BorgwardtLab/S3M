#!/usr/bin/env python3

import csv
import sys

if __name__ == "__main__":
  
  time_series = []
  labels      = []

  with open( sys.argv[1] ) as f:
    reader = csv.reader(f)
    for row in reader:

      # Skip comments; some versions of our data files make use of them
      # in order maintain provenance information.
      if row[0].startswith("#"):
        continue

      time_series.append( row[1:] )
      labels.append( float(row[0]) )

  new_labels = []
  for label in labels:
    if label == 1.0:
      new_labels.append(0.0)
    else:
      new_labels.append(1.0)

  labels = new_labels

  for label, series in zip(labels, time_series):
    sys.stdout.write("{}".format(label))
    for value in series:
      sys.stdout.write(",{}".format(value))
    sys.stdout.write("\n")
