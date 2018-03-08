#!/usr/bin/env python3
#
# preprocess_time_series.py: Performs pre-processing operations for time
# series, such as shortening or smoothing (if desired).
#
# Input:  CSV file containing the time series; the first value is used as
#         the label.
# Output: Pre-processed time series in CSV format
#
# All output will be written to `stdout`.

import sys

# TODO: make configurable
max_length = 60

if __name__ == "__main__":

  all_time_series = []
  labels          = []

  with open(sys.argv[1]) as f:
    for line in f:
      values = [ float(x) for x in line.split(",") ]
      label  = values[0]
      values = values[1:len(values)] # Drop the label of the time series; this
                                     # is the convention that we use---similar
                                     # to the UCR data.

      labels.append(label)
      all_time_series.append( values )

  all_time_series = [ time_series[:max_length] for time_series in all_time_series ]

  ######################################################################
  # Output
  ######################################################################

  for index, time_series in enumerate(all_time_series):
    print("{},{}".format(labels[index], ",".join([str(x) for x in time_series])))
