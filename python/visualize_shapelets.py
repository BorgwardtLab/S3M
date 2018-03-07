#!/usr/bin/env python3
#
# visualize_shapelets.py: Visualizes the shapelets obtained by our
# method by drawing them on top of each other.
#
# Input:  JSON file with shapelets
# Output: None

import json
import math
import sys

import numpy             as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
  with open(sys.argv[1]) as f:
    data = json.load(f)

  shapelets = data["shapelets"]
  p_values  = []

  for shapelet in shapelets:
    p_values.append( shapelet["p_val"] )

  p_min = math.log( min(p_values) )
  p_max = math.log( max(p_values) )

  for shapelet in shapelets:
    s     = shapelet["shapelet"]
    p     = shapelet["p_val"]
    p     = math.log(p)

    if p_max != p_min:
      alpha = max( (p_max - p) / (p_max - p_min), 0.05 )
    else:
      alpha = 1.0

    plt.plot(s, 'k', alpha=alpha, linewidth=5*alpha)

  plt.show()
