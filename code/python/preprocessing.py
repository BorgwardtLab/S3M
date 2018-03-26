#!/usr/bin/env python3

import pandas as pd
import numpy as np

from datetime import datetime
import csv
import json

import os

from sklearn.model_selection import train_test_split
import logging
logging.basicConfig(level=logging.DEBUG)
np.random.seed(42)

cached = {}
for word in ['resprate', 'sysbp', 'heartrate']:
    print(word)
    resp_data_cases = pd.read_csv( "../../data/MIMIC/query11_case_{}_subjectID_v5.csv".format(word) )
    resp_data_cases['chart_time'] = resp_data_cases['chart_time'].apply( lambda x: datetime.strptime(x, "%Y-%m-%d %H:%M:%S") )

    resp_data_control = pd.read_csv( "../../data/MIMIC/query11_control_{}_subjectID_v5.csv".format(word) )
    resp_data_control['chart_time'] = resp_data_control['chart_time'].apply( lambda x: datetime.strptime(x, "%Y-%m-%d %H:%M:%S") )
    cached[word] = { 'cases': resp_data_cases, 'controls': resp_data_control }


max_length = 150

for word in ['resprate', 'sysbp', 'heartrate']:
    res = {}
    data_cases = cached[word]['cases']
    data_control = cached[word]['controls']
    
    num_control_stays = len( data_control['icustay_id'].unique() )
    num_cases_stays = len( data_cases['icustay_id'].unique() )
    
    logging.info( "Found {} controls".format( num_control_stays ) )
    logging.info( "Found {} cases".format( num_cases_stays ) )


    count = 0
    val_count = 0
    unitstay_ids = []
    with open('../../data/MIMIC/Train_{}_seed_42_v5.csv'.format(word), 'w') as train_file:
        train_writer = csv.writer(train_file, delimiter=',')
        with open('../../data/MIMIC/Test_{}_seed_42_v5.csv'.format(word), 'w') as test_file:
            test_writer = csv.writer(test_file, delimiter=',')

            #Write controls
            logging.info( "Randomly sample {} from controls".format(num_cases_stays) )
            id_s = np.random.choice( data_control['icustay_id'].unique(), num_cases_stays, False) # upsampeln mit TRUE?

            X_train, X_test, y_train, y_test = train_test_split(id_s, np.repeat([0], num_cases_stays), test_size=0.33, random_state=42)

            for icustay_id in X_train:
                pat = data_control.query( "icustay_id == @icustay_id" )
                pat.drop_duplicates(subset=['chart_time'], inplace=True)
                pat.set_index( 'chart_time', inplace=True )
                pat.resample('30min').asfreq().ffill().bfill()

                #res[ str(icustay_id) ] = { 'case': 0, 'values': pat[word].values[:max_length].tolist(), 
                #                          "chart_time": pat.index.values[:max_length].tolist() }

                train_writer.writerow( np.concatenate( [[0], pat[word].values[:max_length]] ) )

            for icustay_id in X_test:
                pat = data_control.query( "icustay_id == @icustay_id" )
                pat.drop_duplicates(subset=['chart_time'], inplace=True)
                pat.set_index( 'chart_time', inplace=True )
                pat.resample('30min').asfreq().ffill().bfill()

                #res[ str(icustay_id) ] = { 'case': 0, 'values': pat[word].values[:max_length].tolist(), 
                #                           "chart_time": pat.index.values[:max_length].tolist() }

                test_writer.writerow( np.concatenate( [[0], pat[word].values[:max_length]] ) )

            logging.info( "Wrote {} controls in training, {} in test set.".format(len(X_train), len(X_test)) )
                
            #Write cases
            logging.info( "Writing cases" )
            id_s = data_cases['icustay_id'].unique()
            X_train, X_test, y_train, y_test = train_test_split(id_s, np.repeat([1], num_cases_stays), test_size=0.33, random_state=42)

            for icustay_id in X_train:
                pat = data_cases.query( "icustay_id == @icustay_id" )
                pat.drop_duplicates(subset=['chart_time'], inplace=True)
                pat.set_index( 'chart_time', inplace=True )
                pat.resample('30min').asfreq().ffill().bfill()

                #res[ str(icustay_id) ] = { 'case': 1, 'values': pat[word].values[:max_length].tolist(),
                #                         "chart_time": pat.index.values[:max_length].tolist() }
                train_writer.writerow( np.concatenate( [[1], pat[word].values[:max_length]] ) )

            for icustay_id in X_test:
                pat = data_cases.query( "icustay_id == @icustay_id" )
                pat.drop_duplicates(subset=['chart_time'], inplace=True)
                pat.set_index( 'chart_time', inplace=True )
                pat.resample('30min').asfreq().ffill().bfill()

                #res[ str(icustay_id) ] = { 'case': 1, 'val': 0, 'values': pat[word].values[:max_length].tolist(),
                #                          "chart_time": pat.index.values[:max_length].tolist() }
                test_writer.writerow( np.concatenate( [[1], pat[word].values[:max_length]] ) )
                
            logging.info( "Wrote {} cases in training, {} in test set.".format(len(X_train), len(X_test)) )

    #with open("../data/MIMIC/sepsis/icustay_ids_{}_v5_seed24.json".format(word), 'w') as f:
    #    json.dump(res, f)
