#!/usr/bin/env python3
#
# smooth_time_series.py: This is a demo script for pre-processing time
# series by applying *exponential weighted averages*. We do *not* make
# use of this technique in the paper but provide it here as an example
# for pre-processing techniques.
#
# Input:  CSV file containing the time series; the first value is used as
#         the label.
# Output: Pre-processed time series in CSV format
#
# All output will be written to `stdout`.

import numpy  as np
import pandas as pd

import sys

if __name__ == "__main__":

  ts_all = []
  labels = []

  with open(sys.argv[1]) as f:
    for line in f:
      values = [ float(x) for x in line.split(",") ]
      label  = values[0]
      values = values[1:len(values)] # Drop the label of the time series; this
                                     # is the convention that we use---similar
                                     # to the UCR data.

      labels.append(label)
      ts_all.append( pd.Series(values) )

  for index, ts in enumerate(ts_all):
    values = ts.ewm(span=5).mean()
    print(str(labels[index])+","+",".join([str(x) for x in values]))
