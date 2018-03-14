#!/usr/bin/env python3
#
# embed_distance_matrix.py: Given a distance (dissimilarity) matrix,
# performs multidimensional scaling and embeds the matrix in 2D. The
# embedding can be used to gauge the dissimilarity of shapelets, and
# may be used as precursor to clustering.
#
# Input:  Dissimilarity matrix
# Output: Coordinates of the embedding
#
# The output will be written to `stdout`.

import numpy as np
import sys

from sklearn.manifold import MDS

if __name__ == "__main__":
  data      = np.loadtxt(sys.argv[1], dtype="float32")
  mds       = MDS(dissimilarity="precomputed", n_jobs=-1)
  embedding = mds.fit_transform(data)

  np.savetxt(sys.stdout, embedding, fmt="%.8f")
