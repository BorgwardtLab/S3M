#!/usr/bin/env python3
#
# analyse_variability.py: Given a data set, analysis the variability of
# all time series. Variability is defined as differences along the time
# axis.
#
# Input:  CSV file with time series
# Output: Set of "variability time series"
#
# The output will be written to `stdout`.

import argparse
import csv
import sys

import numpy as np

def variability(time_series):
  """
  Calculates the variability of a given time series.
  """

  data = []
  for index, y in enumerate(time_series):
    if index > 0:
      data.append( y - time_series[index-1] )

  return data

def format_series(time_series):
  """
  Formats a time series as a string for output.
  """

  data = ""
  for x,y in enumerate(time_series):
    data += "{}\t{}\n".format(x,y)

  data += "\n"
  return data

if __name__ == "__main__":
  time_series = []
  labels      = []

  parser = argparse.ArgumentParser(description="Time Series Variability Analysis")

  parser.add_argument("input",
    metavar  = "INPUT",
    help     = "Input file"
  )

  parser.add_argument("-s", "--summarize",
    type     = bool,
    required = False,
    help     = "Calculate variability summaries"
  )

  arguments  = parser.parse_args()
  input_file = arguments.input

  with open(input_file) as f:
    reader = csv.reader(f)
    for row in reader:

      # Skip comments; some versions of our data files make use of them
      # in order maintain provenance information.
      if row[0].startswith("#"):
        continue

      time_series.append( [ float(x) for x in row[1:] ] )
      labels.append( float(row[0]) )

  cases    = [ series for index, series in enumerate(time_series) if labels[index] == 1.0 ]
  controls = [ series for index, series in enumerate(time_series) if labels[index] != 1.0 ]


  if arguments.summarize:
    variability_cases = set()
    for series in cases:
      variability_cases.update(variability(series))

    variability_controls = set()
    for series in controls:
      variability_controls.update(variability(series))

    print(variability_cases)
    print("\n")
    print(variability_controls)
  else:
    for series in cases:
      sys.stdout.write(format_series(variability(series)))

    sys.stdout.write("\n\n")

    for series in controls:
      sys.stdout.write(format_series(variability(series)))
