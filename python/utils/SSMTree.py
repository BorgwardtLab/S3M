import numpy as np

# OS imports
from os.path import exists, join
from os import makedirs, system

# SKLearn imports
from sklearn import tree
from sklearn.ensemble import RandomForestClassifier

# S3M imports
from utils.SSMHelper import SSMHelper
from sklearn.base import BaseEstimator, TransformerMixin
from utils.Distance import distance

# Misc imports
import csv
import json
import logging
import concurrent.futures

class SSMTransform(TransformerMixin):
    
	def __init__( self, min_len: int, max_len: int, stride: int = 1, standardize: bool = True, disable_pruning: bool = False, remove_duplicates: bool = False, k: int = 0, name: str = None, num_cpus: int = 1 ):
		self.dist = distance

		self.min_len = min_len
		self.max_len = max_len
		self.stride = stride
		self.significant_shapelets = []
		self.shapelet_pos_dict = {}
		self.disable_pruning = disable_pruning
		self.remove_duplicates = remove_duplicates
		self.standardize = standardize
		self.k = k

		if name == None:
			raise ValueError( "Name not specified. Please specify a name for your experiment (e.g. eICU_Data_run1)" )

		self.name = name
		self.num_cpus = num_cpus
		self.ssm_helper = SSMHelper( num_cpus=num_cpus )


	def fit_transform(self, X: list, y: list):
		"""Generates all candidate shapelets of for a time series dataset X
		and represents each time series as a vector of length |self.significant_shapelets|.
		Each entry is the minimum distance to respective shapelet to the time series
		"""
		if not isinstance(X, np.ndarray):
			self.X = np.array( X )
		else:
			self.X = X

		self.fit( X, y )
		return self.transform( X )


	def fit(self, X, y):
	    # Merge y and X and save for usage in s3m
		X_y_merged = [ np.concatenate([[ y[idx] ], X[ idx ].tolist()]).tolist() for idx in range(len(X)) ]

		result_path="./s3m_results"
		tmp_path="./s3m_tmp"
		if not exists(tmp_path):
			makedirs(tmp_path)

		if not exists(result_path):
			makedirs(result_path)	

		input_file = join( tmp_path, self.name )
		with open( input_file, 'w' ) as f:
			writer = csv.writer( f, delimiter=',' )
			writer.writerows( X_y_merged )

		
		output_file = join( result_path, "{}.json".format(self.name) )
            
		# Run ssm
		disable_pruning = "-p" if self.disable_pruning else ""
		remove_duplicates = "-r" if self.remove_duplicates else ""
		standardize = "--standardize" if self.standardize else ""

		logging.info( "Running S3M with {}".format("../build/s3m {} {} {} -m {} -M {} -s {} -l 0 -k {} -i {} -o {}".format( standardize, disable_pruning, remove_duplicates, self.min_len, self.max_len, self.stride, self.k, input_file, "{}.json".format(self.name) )) )		
		system( "../build/s3m {} {} {} -m {} -M {} -s {} -l 0 -k {} -i {} -o {}".format( standardize, disable_pruning, remove_duplicates, self.min_len, self.max_len, self.stride, self.k, input_file, output_file ) )

		with open( output_file, 'r' ) as f:
			s3m_results = json.load( f )
			logging.info( "{} significant shapelets found.".format( len(s3m_results['shapelets'] ) ) )
			self.significant_shapelets = [ x['shapelet'] for x in s3m_results['shapelets'] ]

	def transform(self, X: list):
		if not isinstance(X, np.ndarray):
		    self.X = np.array( X )
		else:
		    self.X = X

		# Determine threshold for each shapelet individually 
		self.chunked_shapelets = np.array_split( self.significant_shapelets, self.num_cpus )
		self.chunked_distances = {}

		shapelet_distances = []
		with concurrent.futures.ProcessPoolExecutor(max_workers=self.num_cpus) as executor:
			futures = [executor.submit(self._calc_distance_parallel, { 'shapelets': self.chunked_shapelets[idx], 'data': X, 'chunk_idx': idx } )
		           for idx in range(len(self.chunked_shapelets))]
    
		for future in concurrent.futures.as_completed(futures):
			shapelet_distances.append(future.result())

		# Create a class variable (self.shapelets) that has all significant shapelets in the order in which they have been processed.
		# This order is crucial as it is the feature list of each time series
		transformed_X = np.zeros( ( len(X), len(self.significant_shapelets) ) )

		last_shapelet_idx = 0
		for i, res_dict in enumerate(shapelet_distances):
			current_shapelets = self.chunked_shapelets[ res_dict['chunk_idx'] ]

			for a in range( res_dict['distance_list'].shape[1] ):
				
				if len( self.shapelet_pos_dict ) != len(self.significant_shapelets):
					transformed_X[:,len(self.shapelet_pos_dict)] = res_dict['distance_list'][:,a]
					self.shapelet_pos_dict[ str(current_shapelets[a]) ] = len(self.shapelet_pos_dict)
				else:
					transformed_X[:,self.shapelet_pos_dict[str(current_shapelets[a])]] = res_dict['distance_list'][:,a]
					#transformed_X = np.insert(transformed_X, self.shapelet_pos_dict[str(current_shapelets[a].tolist())], res_dict['distance_list'][:,a], axis=1)


		return transformed_X
	 

	def _calc_distance_parallel( self, args ):
		logging.info( "Calculate distances for shapelet chunk {}".format( args['chunk_idx'] ) )
		shapelets = args['shapelets']
		data = args['data']

		distance_list = []
		for ts in data:
			shapelet_distances = []
			for i, shapelet in enumerate(shapelets):
				dist = self._subsequence_dist( ts, shapelet )
				shapelet_distances.append( dist )
			distance_list.append( shapelet_distances )

		# Assert right size
		assert np.array(distance_list).shape == ( len(data), len(shapelets) )

		return { 'chunk_idx': args['chunk_idx'], 'distance_list': np.array(distance_list) }

	def _subsequence_dist(self, time_series: list, subsequence: np.ndarray, idx: int=-1, complete: bool = False) -> float:
		"""
		Calculates the distance of a subsequence to the given time
		series, while abandoning distance calculations early if no
		improvement over the current distance estimate can be made
		for the pattern.

		Note that this function assumes that distance calculations
		are *additive*. When using the Euclidean distance, one has
		to perform an additional squaring operation.

		:param time_series: Time series for which the distance is to be calculated
		:param subsequence: Subsequence, i.e. candidate pattern
        :param idx: Index of respective time series
        :param complete: Whether to return additional information
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
        
class SSMTree(BaseEstimator):
    
	def __init__( self, min_len: int, max_len: int, stride: int = 1, standardize: bool = True, disable_pruning: bool = False, remove_duplicates: bool = False, k: int = 0, name: str = None, num_cpus: int = 1 ):
		self.ssm_transformer = SSMTransform( min_len=min_len, max_len=max_len, stride=stride, standardize=standardize, disable_pruning=disable_pruning, remove_duplicates=remove_duplicates, k=k, name=name, num_cpus=num_cpus )

	def fit( self, X:list, y: list ):
		self.y_train = y
		self.x_train_transformed = self.ssm_transformer.fit_transform( X, y )

		self.clf = RandomForestClassifier()
		self.clf = self.clf.fit(self.x_train_transformed, self.y_train)

	def predict( self, X:list ):
		self.x_test_transformed = self.ssm_transformer.transform( X )
		return self.clf.predict( self.x_test_transformed )
