#!/usr/bin/env python3

import argparse
from utils.ShapeletEval import ShapeletEval
import pandas as pd
import numpy as np
import csv

from sklearn.model_selection import train_test_split
from os.path import join, exists
from os import makedirs
import logging
import json

import logging
logging.basicConfig(level=logging.INFO)

if __name__ == '__main__':

	parser = argparse.ArgumentParser(description='Evaluate Shapelets')
	parser.add_argument('-n', '--name', nargs=1, required=True, 
                    help='Name for resulting files', dest='name')

	parser.add_argument('-i', '--input', nargs=1, required=True, 
            help='Path to json file', dest='input')

	parser.add_argument('-l', '--label-index', required=True, default=0,
            help='Index of label in time series', dest='label_idx', type=int)

	parser.add_argument('-k', '--keep', required=True,
            help='Maximum number of shapelets to keep (0 = unlimited)', dest='k', type=int)

	parser.add_argument('-tr', '--train', nargs=1, required=True, 
                    help='Filename of Training set', dest='train')

	parser.add_argument('-te', '--test', nargs=1, required=True, 
                help='Filename of Test set', dest='test')

	parser.add_argument('-o', '--out_dir', nargs=1, required=True, 
            help='Output directory', dest='output_dir')
	
	args = parser.parse_args()
	output_dir = args.output_dir[0]
	name = args.name[0]

	metrics_output = output_dir

	#Create dir if not exist
	if not exists( metrics_output ):
		makedirs( metrics_output )

	train = args.train[0]
	test = args.test[0]
	label_idx = args.label_idx

	json_input = args.input[0]

	y_train = []
	X_train = []
	logging.info( "Reading Training File for Evaluation" )
	with open(train, 'r') as train_file:
		reader = csv.reader(train_file, delimiter=',')
		if label_idx == 0 or label_idx == len( np.array(row).shape[1] - 1 ):
			for row in reader:
				X_train.append( (np.array(row[1:]).astype(float)) )
				y_train.append( row[ label_idx ] )
		else:
			raise ValueError( "Label is not located in the first or last column of the data. Please preprocess data accordingly." )

	y_train = np.array(y_train).astype(float).astype(int)
	X_train = np.array(X_train)

	y_test = []
	X_test = []
	logging.info( "Reading Test File for Evaluation" )
	with open(test, 'r') as test_file:
		reader = csv.reader(test_file, delimiter=',')
		if label_idx == 0 or label_idx == len( np.array(row).shape[1] - 1 ):
			for row in reader:
				X_test.append( (np.array(row[1:]).astype(float)) )
				y_test.append( row[ label_idx ] )
		else:
			raise ValueError( "Label is not located in the first or last column of the data. Please preprocess data accordingly." )

	y_test = np.array(y_test).astype(float).astype(int)
	X_test = np.array(X_test)

	#Some labels might come in an unusual format (like UCR), put into 0, ..., n format
	#Train
	range_1 = np.max(y_train) - np.min(y_train)
	zero_to_one_normalized = (y_train - np.min( y_train ) ) / float(range_1)
	y_train_normalized = (zero_to_one_normalized * range_1)

	if 2 in y_train_normalized:
		np.place(y_train_normalized, y_train_normalized == 2, 1.)

	#Test
	range_1 = np.max(y_test) - np.min(y_test)
	zero_to_one_normalized = (y_test - np.min( y_test) ) / float(range_1)
	y_test_normalized = (zero_to_one_normalized * range_1)

	if 2 in y_test_normalized:
		np.place(y_test_normalized, y_test_normalized == 2, 1.)

	logging.info( "Reading {}".format( json_input ) )
	json_obj = json.load( open(json_input))
	parameters = json_obj['parameters']

	#Evaluate only top k shapelets. If k == 0 evaluate all shapelets
	k = np.min([args.k, len(json_obj['shapelets'])])
	if k == 0:
		k = len(json_obj['shapelets'])

	logging.info( "Evaluating metrics and creating visualization for top {} shapelets.".format(k) )

	eval_res = []
	se = ShapeletEval(X_train, y_train_normalized, X_test, y_test_normalized)
	for i, pattern in enumerate(json_obj['shapelets'][:k]):
		shapelet = pattern['shapelet']
		threshold = pattern['threshold']

		p_val = pattern['p_val']
		metrics = se.evaluate(shapelet, threshold, int(y_train_normalized[pattern['index']]))
		eval_res.append( metrics )
		
	pd.DataFrame(eval_res).to_csv(  join(metrics_output, "{}_metrics.csv".format(name) ) , index=False )

