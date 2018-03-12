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


	def evaluate_parallel( self, shapelets: list ):
		logging.info("Evaluate in Parallel following shapelets")
		logging.info( "{}".format(shapelets) )
		res = []
		for shapelet in shapelets:
			res.append( self.evaluate(shapelet) )
		return res

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
			'acc': accuracy_score(predictions, self.y_test),
			'f1': f1_score( predictions, self.y_test ),
			'prec': precision_score(predictions, self.y_test),
			'recall': recall_score(predictions, self.y_test),
			'rand': self.rand_score(self.y_test, predictions),
			'p_val': p_val,
			'contingency': contingency_table.flatten().tolist()[0],
			'shapelet': shapelet.tolist(),
			'distances': distances_on_test} 

		return results

	def get_shapelet_auc(self, X_test: np.ndarray, y_test: np.ndarray, shapelet: list):
		distances_on_test = self._get_distances( shapelet, X_test )
		thresholds = self._get_splitpoints( distances_on_test )

		sens = []
		spec = []

		for threshold in sorted( thresholds ):
			predictions= []

			left_indices = np.argwhere(distances_on_test <= threshold).ravel()
			right_indices = np.argwhere(distances_on_test > threshold).ravel()
			if sum( np.take( y_test, left_indices ) ) >= sum( np.take( y_test, right_indices ) ):
				left_class = 1
			else:
				left_class = 0

			# Distances are sorted according to the original
			# ordering of the time series.
			for ts_index, dist in enumerate(distances_on_test):
				if dist <= threshold:
					predictions.append( left_class )
				else:
					predictions.append( int(not left_class) )

			TN, FP, FN, TP = confusion_matrix( y_test, predictions ).ravel()

			sens.append( ShapeletEval.get_sens(TP, FN) )
			spec.append( 1 - ShapeletEval.get_spec(TN,FP) )

		return auc( sorted(spec), sorted(sens) )

	@staticmethod
	def get_sens( TP: int, FN: int ):
		return TP/float(TP+FN)

	@staticmethod
	def get_spec( TN: int, FP: int ):
		return TN/float(TN+FP)

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

	@staticmethod
	def rand_score(true, predicted):
		a = 0
		b = 0
		for i in range(len(true)):
			for n in range(i+1, len(predicted)):
				if true[i] == true[n] and predicted[i] == predicted[n]:
					a += 1
				if true[i] != true[n] and predicted[i] != predicted[n]:
					b += 1

		return (a+b)/float(binom( len(true), 2 ))

	def _subsequence_dist_classic(self, time_series: list, subsequence: np.ndarray, complete: bool = False) -> float:
		"""

		:param time_series: Time series to to which the distance is calculated
		:param subsequence: Subsequence
		:param complete: Whether to return position and sequence
		:return:
		"""
		if len(time_series) < len(subsequence):
			temp = time_series
			time_series = subsequence
			subsequence = temp
		min_dist = np.inf
		min_dist_pos = 0
		min_dist_seq = []
		for window_start in range(0, len(time_series) - len(subsequence) + 1):
			current_subseq = time_series[window_start:window_start + len(subsequence)]
			current_dist = self.dist(np.array(subsequence), current_subseq)
			if current_dist < min_dist:
				min_dist = current_dist
				min_dist_pos = window_start
				min_dist_seq = current_subseq
		if complete:
			return min_dist, min_dist_pos, np.array(min_dist_seq)
		else:
			return min_dist

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

	def create_shapelet_plots( self, pattern: dict, dir: str = './', x_label: str = 'x', y_label: str = 'y', title: str ='Shapelet Plot', file="Shapelet_Plot", figsize: tuple=(12,2), dpi: int = 300 ):
		"""
		Creates one plot each for positives and negatives in training and test set.

		:param pattern: A dictionary that at least contains the subsequence and its position 
		"""
		shapelet = pattern['shapelet']
		#cp_val = pattern['p_val']
		#threshold = pattern['threshold']

		current_set = "Test Set"
		X = self.X_test
		y = self.y_test

		plt.close('all')
		fig, ax = plt.subplots( figsize=figsize, dpi=dpi )
		ax.set(ylabel=y_label, title="{}: {} Positives".format(title, current_set))
		ax.set_xlabel(x_label)

		#Plot time series and highlight sequence with min distance to pattern
		for ts in X[y == 1]:
		    min_dist, min_dist_pos, min_dist_seq, idx = self._subsequence_dist( ts, shapelet, complete=True )

		    ax.plot(np.arange(0, len(ts)), ts, color='green', alpha=0.1)
		    ax.plot(np.arange(min_dist_pos, min_dist_pos+len(min_dist_seq)), min_dist_seq, color='green')

		#Plot Pattern
		x = np.arange(pattern['pos'], pattern['pos'] + len(shapelet) )
		ax.plot( x, shapelet, color='orange', linewidth=3.5 )

		ax.grid()
		if dir:
			plt.savefig( join( dir, "{}_{}_Positives.png".format(file, current_set ) ) )
		else:
			plt.show()

		plt.close('all')
		fig, ax = plt.subplots( figsize=figsize, dpi=dpi )
		ax.set(ylabel=y_label, title="{}: {} Positives".format(title, current_set))
		ax.set_xlabel(x_label)

		#Plot time series and highlight sequence with min distance to pattern
		for ts in X[y == 0]:
		    min_dist, min_dist_pos, min_dist_seq, idx = self._subsequence_dist( ts, shapelet, complete=True )

		    ax.plot(np.arange(0, len(ts)), ts, color='purple', alpha=0.1)
		    ax.plot(np.arange(min_dist_pos, min_dist_pos+len(min_dist_seq)), min_dist_seq, color='purple')

		#Plot Pattern
		x = np.arange(pattern['pos'], pattern['pos'] + len(shapelet) )
		ax.plot( x, shapelet, color='orange', linewidth=3.5 )

		#Determine evaluation metrics
		'''predictions = self.predict_by_shapelet( shapelet, threshold, X_test )

		acc = accuracy_score(predictions, y_test)
		prec = precision_score(predictions, y_test) 
		recall = recall_score(predictions, y_test) 
		rand = get_rand(y_test, predictions) 

		accuracies.append(acc)
		rand_indices.append(rand)
		p_vals.append(p_val)'''

		ax.grid()
		if dir:
			plt.savefig( join( dir, "{}_{}_Negatives.png".format(file, current_set ) ) )
		else:
			plt.show()