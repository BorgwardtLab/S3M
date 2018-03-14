import numpy as np 
from utils.Distance import distance
from scipy.stats import chi2_contingency
import matplotlib 
matplotlib.use('agg')

import matplotlib.pyplot as plt
from os.path import join

import pandas as pd
from scipy.special import binom
import logging

from sklearn.metrics import accuracy_score, precision_score, recall_score, confusion_matrix, f1_score, auc


class ShapeletEval(object):

	def __init__( self, X_train: np.ndarray, y_train: np.ndarray, X_test: np.ndarray, y_test: np.ndarray, dist: 'function' = distance ):
		self.X_train = X_train
		self.X_test = X_test
		self.dist = dist
		self.y_test = y_test
		self.y_train = y_train

	def evaluate( self, shapelet: list, threshold: float, gt_label: int, with_distances: bool=False ):
		"""
		Evaluates a shapelet at all possible splitpoints from the training set on the test set. 

		:return: A list of all metrics (accuracy, precision, recall, rand index) together with respective split threshold, contingency table, and p value
		"""
		if type(shapelet) != np.ndarray:
			shapelet = np.array( shapelet ) 

		distances_on_test = self._get_distances( shapelet, self.X_test )
		if np.all(self.X_test == self.X_train):
			splitpoints_on_train = self._get_splitpoints( distances_on_test )
		else:
			splitpoints_on_train = self._get_splitpoints( self._get_distances( shapelet, self.X_train ) )

		logging.info( "Calculate evaluation metrics for all splitpoints and test data." )
		sens = []
		spec = []
		splitpoint = [threshold]
		predictions = []

		# Ensures that the matrix always contains valid values;
		# else we have to correct for zero rows/columns
		a_s = 1
		b_s = 1
		c_s = 1
		d_s = 1

		left_indices = np.argwhere(distances_on_test <= splitpoint).ravel()
		right_indices = np.argwhere(distances_on_test > splitpoint).ravel()
		if sum( np.take( self.y_test, left_indices ) ) >= sum( np.take( self.y_test, right_indices ) ):
			left_class = gt_label
		else:
			left_class = gt_label

		for ts_index, distance in enumerate( distances_on_test ):
			if distance <= splitpoint:
				predictions.append( left_class )
				if self.y_test[ts_index] == int(not left_class):
					d_s += 1
				else:
					a_s += 1
			else:
				if self.y_test[ts_index] == int(not left_class):
					c_s += 1
				else:
					b_s += 1
				predictions.append( int(not left_class) )

		contingency_table = np.matrix( [ [a_s, b_s], [d_s, c_s] ] ).astype(int)
		p_val = chi2_contingency(contingency_table, correction=False)[1]
		
		results = {'threshold': splitpoint[0],
			'acc': np.max( [accuracy_score(predictions, self.y_test), 1-accuracy_score(predictions, self.y_test)]),
			'p_val': p_val,
			'contingency': contingency_table.flatten().tolist()[0],
			'shapelet': shapelet.tolist(),
			'distances': distances_on_test} 

		return results

	def _get_distances( self, shapelet: list, reference: list ):
		logging.debug( "Calculate distances." )
		distances = []
		for i, ts in enumerate(reference):
			distances.append( self._subsequence_dist( ts, shapelet, i ) )
		return distances

	def _get_splitpoints( self, distances: list ):
		logging.debug( "Get splitpoints." )
		idx_distances = np.argsort(distances)
		return [(distances[idx_distances[i]] + distances[idx_distances[i + 1]]) / float(2) for i, idx in 
			enumerate(idx_distances) if i < len(idx_distances) - 1]

	def _subsequence_dist(self, time_series: list, subsequence: np.ndarray, idx: int=-1, complete: bool = False) -> float:
		"""
		Calculates the distance of a subsequence to the given time
		series, while abandoning distance calculations early if no
		improvement over the current distance estimate can be made
		for the pattern.

		Note that this function assumes that distance calculations
		are *additive*. When using the Euclidean distance, one has
		to perform an additional squaring operation.

		:param time_series_idx: Index of time series for which the distance is to be calculated
		:param subsequence: Subsequence, i.e. candidate pattern
		:return: Distance of subsequence to given time series
		"""
		if len(time_series) < len(subsequence):
			temp = time_series
			time_series = subsequence
			subsequence = temp
		min_dist = np.inf
		min_dist_pos = 0
		min_dist_seq = []
		for window_start in range(0, len(time_series) - len(subsequence) + 1):
			stop = False
			# Elementwise distance for early abandon
			sum_dist = 0
			for i, m in enumerate(subsequence):
				current_subseq = time_series[window_start:window_start + len(subsequence)]
				sum_dist += self.dist(m, time_series[window_start + i])
				if sum_dist >= min_dist:
					stop = True
					break
			if not stop:
				min_dist = sum_dist
				min_dist_pos = window_start
				min_dist_seq = current_subseq

		if complete:
			return [min_dist, min_dist_pos, np.array(min_dist_seq), idx]
		else:
			return min_dist