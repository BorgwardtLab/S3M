#!/usr/bin/env python3
#
# visualize_shapelets.py: Visualizes the shapelets obtained by our
# method by drawing them on top of each other.
#
# Input:  JSON file with shapelets
# Output: None

import json
import sys

import numpy             as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
  with open(sys.argv[1]) as f:
    data = json.load(f)

  shapelets = data["shapelets"]

  for shapelet in shapelets:
    s = shapelet["shapelet"]
    plt.plot(s)

  plt.show()
