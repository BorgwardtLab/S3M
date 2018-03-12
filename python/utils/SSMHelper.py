import numpy as np
from utils.Distance import distance
import concurrent.futures
import csv
from sklearn.model_selection import cross_val_score
from utils.ShapeletEval import ShapeletEval
from sklearn.model_selection import StratifiedKFold

import matplotlib 
matplotlib.use('agg')
import matplotlib.pyplot as plt

import os
from os.path import join, exists
import json
import logging

#logging.basicConfig( level=logging.DEBUG )

class SSMHelper(object):

	def __init__( self, num_cpus: int = 1 ):
		self.dist = distance
		self.shapelets = []
		self.thresholds = []
		self.num_cpus = num_cpus

	def compare_to_baseline( self, baseline_models: list, train_path: str, test_path: str, result_path: str, name: str, iteration: int, ground_truth: list = [], min_len: int = 10, max_len: int = 10, stride: int = 1, cv: int = 5, scoring: list = ['accuracy', 'precision', 'recall', 'f1']  ):
		
		# Read Training Data
		X_train = []
		y_train = []
		with open(train_path, 'r') as train_file:
			reader = csv.reader(train_file, delimiter=',')
			for row in reader:
				X_train.append( (np.array(row[1:]).astype(float)) )
				y_train.append(np.array(row[0]).astype(float).astype(int))

		# Read Test Data
		X_test = []
		y_test = []
		with open(test_path, 'r') as test_file:
			reader = csv.reader(test_file, delimiter=',')
			for row in reader:
				X_test.append( (np.array(row[1:]).astype(float)) )
				y_test.append(np.array(row[0]).astype(float).astype(int))


		# Evaluate top k shapelets for SSM and basline methods
		k = 10
		result_path="./ssm_tmp"
		if not exists(result_path):
			os.makedirs(result_path)

		#ssm_cross_val_metrics = self.cross_validate_ssm( X_train, y_train, name, result_path, stride, min_len, max_len, cv )

		# Fit ssm
		logging.info( "Train SSM" )
		ssm_shapelets, ssm_parameters = self.run_ssm(train_path, test_path, result_path, name, stride=stride, min_len=min_len, max_len=max_len)
						
		shapelet_eval = ShapeletEval( X_train, y_train, X_train, y_train )


		# Create dict with SSM properties
		ssm = {}
		ssm_distances = []
		ssm_aucs = []
		ssm_p_val = []
		ssm_top_candidates = []
		ssm_acc= []
		ssm_recall=[]
		ssm_prec=[]
		for i, ssm_shapelet in enumerate(ssm_shapelets):
			if i < k:
				ssm_top_candidates.append( ssm_shapelet['shapelet'] )
				ssm_distances.append( self._subsequence_dist( ssm_shapelet['shapelet'], ground_truth ) )
				#ssm_aucs.append( shapelet_eval.get_shapelet_auc(X_test, y_test, ssm_shapelet['shapelet']) )

				all_metrics = shapelet_eval.evaluate( ssm_shapelet['shapelet'] )
				sorted_pvals = sorted( all_metrics, key=lambda x: x['p_val'] )
				ssm_p_val.append( sorted_pvals[0]['p_val'] )
				ssm_acc.append( sorted_pvals[0]['acc'] )
				ssm_recall.append( sorted_pvals[0]['recall'] )
				ssm_prec.append( sorted_pvals[0]['prec'] )
			else:
				break
		#ssm['cv'] = { 'num_folds': cv, 'results': ssm_cross_val_metrics }

		ssm['top_k_gt_distances'] = ssm_distances
		#ssm['top_k_aucs'] = ssm_aucs
		ssm['top_k_pvals']= ssm_p_val
		ssm['top_k_acc']= ssm_acc
		ssm['top_k_prec']= ssm_prec
		ssm['top_k_recall']= ssm_recall
		ssm['top_candidates']= ssm_top_candidates
		
		# Encode data to binary representation
		encoded_train_data = self.fit_transform( X_train, y_train, min_len=min_len, max_len=max_len )
		encoded_test_data = self.transform( X_test )


		# Create dict with SSM properties
		#Fit baseline models and get metrics
		baselines = {}
		shapelet_eval = ShapeletEval( X_train, y_train, X_train, y_train )
		for i, model in enumerate( baseline_models ):
			i = str(i)
			baselines[ i ] = {}

			# Get CV scores
			#scores = self.cross_validate_general( model, encoded_train_data, y_train, n_splits=cv )
			#baselines[ i ]['cv'] = scores

			#scores = cross_val_score(model, X_train, y_train, cv=5, scoring='accuracy')
			#baselines[ i ]['scores'] = scores

			# Fit model
			model.fit( encoded_train_data, y_train )

			# Get coefficents 
			baselines[ i ]['model'] = model
			if hasattr( model, 'coef_' ):
				coef = model.coef_
				if type(coef) == np.ndarray:
					coef = coef.tolist()

				baselines[ i ]['coef'] = coef

				# Get rank of ground truth
				'''abs_rank, rel_rank, closest_dist, closest_shapelet = self.find_ground_truth_rank( self.shapelets, model.coef_[0], ground_truth, shapelet_eval )
																baselines[ i ]['rel_rank_gt'] = str(rel_rank)
																baselines[ i ]['abs_rank_gt'] = str(abs_rank)
																baselines[ i ]['min_dist_gt'] = str(closest_dist)
																baselines[ i ]['min_dist_gt_seq'] = closest_shapelet'''

				# Get distance between top k shapelets by coef
				# Was only necessary for elastic net and lasso, dumb work around
				if i == '2' or i == "3":
					sorted_idx = np.argsort( np.array(model.coef_)**2 )
				else:
					sorted_idx = np.argsort( np.array(model.coef_[0])**2 )

				model_top_k_ss = np.take( self.shapelets, sorted_idx[-k:], axis=0 ) #self.shapelets comes from the fit transform function



				model_distances = []
				model_aucs = []
				model_p_val = []
				model_acc = []
				model_prec = []
				model_recall = []
				model_top_candidates = []
				shapelet_eval = ShapeletEval( X_train, y_train, X_train, y_train )
				for model_shapelet in model_top_k_ss:
					if type(model_shapelet) == np.ndarray:
						model_shapelet = model_shapelet.tolist()
					model_top_candidates.append( model_shapelet )

					model_distances.append( self._subsequence_dist( model_shapelet, ground_truth ) )
					#model_aucs.append( shapelet_eval.get_shapelet_auc(X_test, y_test, model_shapelet) )

					all_metrics = shapelet_eval.evaluate( model_shapelet )
					sorted_pvals = sorted( all_metrics, key=lambda x: x['p_val'] )
					model_p_val.append( sorted_pvals[0]['p_val'] )
					model_acc.append( sorted_pvals[0]['acc'] )
					model_prec.append( sorted_pvals[0]['prec'] )
					model_recall.append( sorted_pvals[0]['recall'] )

				baselines[i]['top_k_gt_distances'] = model_distances
				#baselines[i]['top_k_aucs'] = model_aucs
				baselines[i]['top_k_pvals'] = model_p_val
				baselines[i]['top_k_acc'] = model_acc
				baselines[i]['top_k_prec'] = model_prec
				baselines[i]['top_k_recall'] = model_recall
				baselines[i]['top_candidates'] = model_top_candidates

		#Create plots
		logging.info( "Create Plots" )
		plt.close('all')

		number_of_baselines = len(baselines.keys())

		f, axarr = plt.subplots(number_of_baselines+2, sharex=False, figsize=(7,7))
		axarr[0].plot(np.arange(len(ground_truth)), ground_truth)

		if len(ssm['top_candidates']) > 0:
			for cand in ssm['top_candidates']:
				axarr[1].plot(np.arange(len(cand)), cand)
			#ssm_mss = ssm['top_candidates'][0]
		    
		for i in np.arange( number_of_baselines ):

			for cand in baselines[str(i)]['top_candidates']:
				axarr[i+2].plot(np.arange(len(cand)), cand)

			#top_cand_model_0 = baselines[str(0)]['top_candidates'][ len(baselines[str(0)]['top_candidates'])-1 ]
			
			'''for cand in baselines[str(1)]['top_candidates']:
													axarr[3].plot(np.arange(len(cand)), cand)
									
												for cand in baselines[str(2)]['top_candidates']:
													axarr[4].plot(np.arange(len(cand)), cand)'''

		#top_cand_model_1 = baselines[str(1)]['top_candidates'][ len(baselines[str(1)]['top_candidates'])-1 ]
		#axarr[3].plot(np.arange(len(top_cand_model_1)), top_cand_model_1)

		plt.savefig( join( result_path, 'plots', "{}_{}.png".format(name, iteration)) )

		return baselines, ssm

	def find_ground_truth_rank( self, shapelet_list: list, coef: list, ground_truth: list, shapelet_eval ):

		closest_shapelet = []
		closest_idx = 0
		closest_dist = np.inf
		for i, shapelet in enumerate(shapelet_list):
			d = shapelet_eval._subsequence_dist( shapelet, ground_truth )
			if d < closest_dist:
				closest_shapelet = shapelet
				closest_dist = d
				closest_idx = i

		sorted_coef = np.argsort( np.array(coef)**2 )
		abs_rank = sorted_coef[ closest_idx ]
		rel_rank = sorted_coef[ closest_idx ] / float( len(sorted_coef) )

		return abs_rank, rel_rank, closest_dist, closest_shapelet

	def cross_validate_general( self, model, X_train: np.ndarray, y_train: list, n_splits: int = 5 ):
		if not isinstance(X_train, np.ndarray):
			X_train = np.array(X_train)

		if not isinstance(y_train, np.ndarray):
			y_train = np.array(y_train)

		skf = StratifiedKFold(n_splits=n_splits)

		cross_val_metrics = []
		current_fold = 1
		for train_index, val_index in skf.split(X_train, y_train):
			logging.info( "Fitting model for fold {} with {} training and {} validation samples".format(current_fold, len(train_index), len(val_index)) )
			model.fit( X_train[train_index], y_train[train_index] )

			if hasattr( model, 'coef_' ):
				# Get distance between top k shapelets by coef
				sorted_idx = np.argsort( np.array(model.coef_[0])**2 )

				logging.info( "Choosing most significant shapelet for evaluation metrics" )
				most_sig_shapelet = np.take( self.shapelets, sorted_idx[ len(sorted_idx)-1 ], axis=0 )

				# create instance of shapeletEval with current training/test split
				shapelet_eval = ShapeletEval( X_train[train_index], y_train[train_index], X_train[val_index], y_train[val_index] )
				stats = shapelet_eval.evaluate( most_sig_shapelet )
				# calculate acc, p-val, f2, prec, recall, ...
				cross_val_metrics.append(stats)
			else:
				logging.info( "Encountered classifier without coef_ attribute. Currently not supported for evaluation." )

			current_fold += 1
		return cross_val_metrics


	def cross_validate_ssm( self, X_train: np.ndarray, y_train: list, name: str, result_path: str, stride: int = 1, min_len: int = 10, max_len: int = 10, n_splits: int = 5 ):
		if not isinstance(X_train, np.ndarray):
			X_train = np.array(X_train)

		if not isinstance(y_train, np.ndarray):
			y_train = np.array(y_train)

		skf = StratifiedKFold(n_splits=n_splits)

		cross_val_metrics = []
		current_fold = 1
		for train_index, val_index in skf.split(X_train, y_train):
			# write to file system tmp
			train_x_file_name = "{}-fold_tmp_{}".format(current_fold, name)
			file_path = join( result_path, train_x_file_name )

			X_y_merged = [ np.concatenate([[ y_train[idx] ], X_train[ idx ]]) for idx in train_index ]
			np.savetxt( file_path, X_y_merged, delimiter=',' )

			# run ssm
			logging.info( "Running SSM for fold {} with {} training and {} validation samples".format(current_fold, len(train_index), len(val_index)) )
			os.system("../ssm.sh -m {} -M {} -s {} -r {} -t {} -o {} -n {}".format( min_len, max_len, stride, file_path, file_path, result_path, train_x_file_name) )

			# read resulting shapelets
			with open( join( result_path, "{}.json".format(train_x_file_name) ) , 'r' ) as f:
				ssm_result = json.load( f )

			# create instance of shapeletEval with current training/test split
			shapelet_eval = ShapeletEval( X_train[train_index], y_train[train_index], X_train[val_index], y_train[val_index] )

			logging.info( "{} significant shapelet(s) found.".format(len(ssm_result['shapelets'])) )

			if len(ssm_result['shapelets']) > 0:
				ssm_shapelets_result = sorted( ssm_result['shapelets'], key=lambda x: x['p_val'])

				logging.info( "Choosing most significant shapelet for evaluation metrics" )
				most_sig_shapelet = ssm_shapelets_result[0]['shapelet']
				stats = shapelet_eval.evaluate( most_sig_shapelet )
				# calculate acc, p-val, f2, prec, recall, ...
				cross_val_metrics.append(stats)
				
			current_fold += 1
		return cross_val_metrics

	def run_ssm(self, train_path: str, test_path: str, result_path: str, name: str, min_len: int = 10, max_len: int = 10, stride: int = 1):
		self.filename = "{}_{}_{}".format(name, min_len, max_len )

		os.system("../ssm.sh -m {} -M {} -s {} -r {} -t {} -o {} -n {}".format( min_len, max_len, stride, train_path, test_path, result_path, self.filename) )

		with open( join( result_path, "{}.json".format( self.filename ) ) , 'r' ) as f:
			ssm_result = json.load( f )

		return sorted( ssm_result['shapelets'], key=lambda x: x['p_val']), ssm_result['parameters']


	def generate_candidates( self, data: np.ndarray, min_len: int = 10, max_len: int = 12, stride: int = 1 ):
		data = SSMHelper._get_ndarray(data)
		if max_len == 0:
			max_len = min_len

		print(max_len)
		print(min_len)

		pool = []
		l = max_len
		logging.info("Generate all possible candidates...")
		while l >= min_len:
			for i, time_series in enumerate(data):
				for window_start in range(0, len(time_series) - l + 1, stride):
					as_list = time_series[window_start:window_start + l].tolist()
					if as_list not in pool:
						pool.append(as_list)
			l = l - 1
		logging.info("generated {0} candidates".format(len(pool)))

		return np.array(pool)

	@staticmethod
	def compute_info_content(y):
		"""
		commpute information content info(D).
		"""
		y_copy = y.copy()
		y_copy = np.concatenate([y_copy, [1,0]])
		class_counts = np.unique( y_copy, return_counts=True )[1]

		class_wise_prop = class_counts/float( sum(class_counts) )
		log_prob = np.log2( class_wise_prop )
		return -sum( ( class_wise_prop * log_prob ) )

	def compute_info_a(self, y_split_left, y_split_right):
		"""
		compute conditional information content Info_A(D).
		"""

		left_class = (len(y_split_left) / float(self.dataset_length)) * SSMHelper.compute_info_content( y_split_left )
		right_class = (len(y_split_right) / float(self.dataset_length)) * SSMHelper.compute_info_content( y_split_right )
		return left_class + right_class


	def compute_info_gain(self, y_split_left, y_split_right):
		"""
		compute information gain(A) = Info(D) - Info_A(D)
		"""
		return self.info_content - self.compute_info_a( y_split_left, y_split_right )

	def calc_distance_parallel( self, args ):
		logging.info( "Calculate distances for shapelet chunk {}".format( args['chunk_idx'] ) )
		shapelets = args['shapelets']
		data = args['data']
		y_data = args['y_data']

		result = []
		encoded_data = []
		threshold_list= []
		for i, shapelet in enumerate(shapelets):
			shapelet_distances = []
			for ts in data:
				dist = self._subsequence_dist( ts, shapelet )
				shapelet_distances.append( dist )

			# Get mean splitpoints
			splitpoints = self._get_splitpoints( shapelet_distances )
			shapelet_distances = np.array( shapelet_distances )

			# Iterate over splitpoints to find optimal information gain only if we did not fit data before
			if len( self.chunked_thresholds ) == 0:
				bsf_gain = 0
				bsf_splitpoint = 0
				for splitpoint in splitpoints:
					left_labels = y_data[ shapelet_distances <= splitpoint ]
					right_labels = y_data[ ~(shapelet_distances <= splitpoint) ]

					info_gain = self.compute_info_gain( left_labels, right_labels )
					if info_gain > bsf_gain:
						bsf_gain = info_gain
						bsf_splitpoint = splitpoint

				threshold_list.append( [bsf_splitpoint]  )

			encoded_data.append( shapelet_distances )

			# Simply use mean as threshold
			#if len( self.chunked_thresholds ) == 0:
			#	threshold_list.append( [np.mean( shapelet_distances )]  )
			#encoded_data.append( shapelet_distances )

		threshold_list = np.array(threshold_list) 
		encoded_data = np.array(encoded_data)

		# Resulting matrices have number of shapelet rows and number of time series columns
		assert encoded_data.shape == ( len(shapelets), len(data) )

		if len( self.chunked_thresholds ) == 0:
			encoded_data = (encoded_data > threshold_list).astype(int).T
		else:
			encoded_data = (encoded_data > self.chunked_thresholds[args['chunk_idx']]).astype(int).T

		return { 'chunk_idx': args['chunk_idx'], 'encoded_data': encoded_data, 'threshold_list': threshold_list }


	def _get_splitpoints( self, distances: list ):
		logging.debug( "Get splitpoints." )
		idx_distances = np.argsort(distances)
		return [(distances[idx_distances[i]] + distances[idx_distances[i + 1]]) / float(2) for i, idx in 
			enumerate(idx_distances) if i < len(idx_distances) - 1]

	def fit_transform( self, data: np.ndarray, y_data: list, min_len: int = 10, max_len: int = 12, stride: int = 1 ):
		"""
		Encodes the data set into a binary vector that represents whether a shapelet exists in a time series
		For each shapelet a threshold is determined at which point it is considered to be existent in a TS
		"""
		if not isinstance(data, np.ndarray):
			data = np.array(data)

		if not isinstance(y_data, np.ndarray):
			y_data = np.array(y_data)

		# Calculate information content of data
		self.info_content = self.compute_info_content( y_data )
		self.dataset_length = len(y_data)


		logging.info( "Create threshold matrix based on training data and transform for encoding." )
		data = SSMHelper._get_ndarray(data)
		shapelets = self.generate_candidates( data, min_len, max_len, stride )
		
		encoded_data = []
		threshold_list = []
		# Determine threshold for each shapelet individually 
		self.chunked_shapelets = np.array_split( shapelets, self.num_cpus )
		self.chunked_thresholds = {}

		shapelet_distances = []
		with concurrent.futures.ProcessPoolExecutor(max_workers=self.num_cpus) as executor:
			futures = [executor.submit(self.calc_distance_parallel, { 'shapelets': self.chunked_shapelets[idx], 'data': data, 'y_data': y_data, 'chunk_idx': idx } )
		           for idx in range(len(self.chunked_shapelets))]

		for future in concurrent.futures.as_completed(futures):
			shapelet_distances.append(future.result())

		# Create a class variable (self.shapelets) that has all patterns in the order in which they have been processed 
		for i, res_dict in enumerate(shapelet_distances):
			if i == 0:
				encoded_data = res_dict['encoded_data']
			else:
				encoded_data = np.concatenate( [encoded_data, res_dict['encoded_data']], axis=1 )
			self.shapelets += self.chunked_shapelets[ res_dict['chunk_idx'] ].tolist()
			self.chunked_thresholds[ res_dict['chunk_idx'] ] = res_dict['threshold_list']

		assert len(self.chunked_thresholds) == len(self.chunked_shapelets)
		assert encoded_data.shape == ( len(data), len(self.shapelets) )

		self.train_data = data
		self.encoded_data = encoded_data

		return encoded_data

	def transform( self, data: np.ndarray ):
		"""
		Transforms data based on shapelets that have been found through fit step on training data
		"""
		logging.info( "Fit data based on trained threshold matrix." )

		if not isinstance(data, np.ndarray):
			data = np.array(data)

		if np.all(data == self.train_data):
			print("Same")
			return self.encoded_data
		else:
			shapelet_distances = []
			with concurrent.futures.ProcessPoolExecutor(max_workers=self.num_cpus) as executor:
				futures = [executor.submit(self.calc_distance_parallel, { 'shapelets': self.chunked_shapelets[idx], 'data': data, 'y_data': None, 'chunk_idx': idx } )
			           for idx in range(len(self.chunked_shapelets))]

			for future in concurrent.futures.as_completed(futures):
				shapelet_distances.append(future.result())

			for i, res_dict in enumerate(shapelet_distances):
				if i == 0:
					encoded_data = res_dict['encoded_data']
				else:
					encoded_data = np.concatenate( [encoded_data, res_dict['encoded_data']], axis=1 )

			assert encoded_data.shape == ( len(data), len(self.shapelets) )

			return encoded_data




	def _subsequence_dist(self, time_series: list, subsequence: np.ndarray, idx: int=-1, complete: bool = False) -> float:
		"""
		Calculates the distance of a subsequence to the given time
		series, while abandoning distance calculations early if no
		improvement over the current distance estimate can be made
		for the pattern.

		Note that this function assumes that distance calculations
		are *additive*. When using the Euclidean distance, one has
		to perform an additional squaring operation.

		:param idx: Index of time series for which the distance is to be calculated
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

	def _get_ndarray(list: list) -> np.ndarray:
		return list if not isinstance(list, np.ndarray) else np.array(list)



