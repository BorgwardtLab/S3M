from scipy.spatial.distance import cityblock
from scipy.spatial.distance import sqeuclidean

def distance(seq1, seq2, dist=sqeuclidean):
  """
  Utility function for calculating the distance between two sequences,
  i.e. between two time-series or their subsequences.

  :param seq1: First sequence
  :param seq2: Second sequence
  :param dist: Distance functor to use for calculating distances between points
               along the two series. The functor should be additive, meaning it
               should be possible to calculate the distance by evaluating parts
               of the sequence and adding them together.

               To ensure this, the *squared* Euclidean distance is used as this
               function's default functor.
  :return: Distance between the two sequences
  """
  return dist(seq1, seq2)
