#!/usr/bin/env python3
#
# analyse_distributions.py: analyses the $p$-value distributions of
# a set of result files from S3M.

import collections
import json
import os
import sys

import numpy as np

import matplotlib.pyplot as plt
import seaborn as sns


if __name__ == '__main__':

    distributions = collections.defaultdict(list)

    for filename in sys.argv[1:]:

        basename = os.path.splitext(os.path.basename(filename))[0]

        with open(filename) as f:
            data = json.load(f)
            data = data['shapelets']

            for shapelet in data:
                p_val = float(shapelet['p_val'])
                distributions[basename].append(-np.log10(p_val))

    for name in distributions.keys():
        sns.distplot(distributions[name], bins=10, label=name, axlabel='$p$-value (-log)')

    plt.legend()
    plt.show()
