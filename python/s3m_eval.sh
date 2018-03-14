#!/bin/bash

usage="$(basename "$0") [-h] -i -d -e [-t -n -p -r -m -M -s -l -k -q -f] -- Run S3M and evaluate significant shapelets on a test data set

Available options:
    -h  show this help text
    -i  path to training file
    -d 	identifier (e.g. Heart_rate)

    -t  Merge equal contingency tables
    -n 	Keep only normal p-values
    -p 	Disable pruning criterion
    -r 	Remove duplicates
    -m 	Minimum candidate pattern length (=10)
    -M 	Maximum candidate pattern length (=0)
    -s 	Stride (=1)
    -l 	Index of label in time series (=0)
    -k 	Maximum number of shapelets to keep (0 = unlimited)
    -q 	Standardize data
    -f 	Use default factor and do not apply correction from the paper" 

S3M_OUTDIR="./s3m_out"
OUTDIR="../results"
IDENTIFIER="S3MViz"
MIN_WINDOW=10
MAX_WINDOW=10
STRIDE=1
LABEL_INDEX=0
KEEP=5

while getopts "htnprm:M:s:l:k:i:d:qe:f" option
do
 case "${option}"
 in
 h) echo "$usage"
	exit
	;;
 t) MERGE_TABLE=true;;
 n) KEEP_NORMAL_ONLY=true;;
 p) DISABLE_PRUNING=true;;
 r) REMOVE_DUPLICATES=true;;
 m) MIN_WINDOW=${OPTARG};;
 M) MAX_WINDOW=${OPTARG};;
 s) STRIDE=${OPTARG};;
 l) LABEL_INDEX=${OPTARG};;
 k) KEEP=${OPTARG};;
 i) INPUT=${OPTARG};;
 d) IDENTIFIER=${OPTARG};;
 q) SQUEEZE=true;;
 e) TEST=${OPTARG};;
 f) FACTOR=true;;
 esac
done

#Make $S3M_OUTDIR if it does not exist
if [ ! -d $S3M_OUTDIR ] ; then
	mkdir $S3M_OUTDIR 
fi


#Make $OUTDIR if it does not exist
if [ ! -d $OUTDIR ] ; then
	mkdir $OUTDIR
fi

if [ "$SQUEEZE" == "true" ] ; then
	SQUEEZE="--standardize"
fi

if [ "$MERGE_TABLE" == "true" ] ; then
	MERGE_TABLE="-t"
fi

if [ "$KEEP_NORMAL_ONLY" == "true" ] ; then
	KEEP_NORMAL_ONLY="-n"
fi

if [ "$DISABLE_PRUNING" == "true" ] ; then
	DISABLE_PRUNING="-p"
fi

if [ "$REMOVE_DUPLICATES" == "true" ] ; then
	REMOVE_DUPLICATES="-r"
fi

if [ "$FACTOR" == "true" ] ; then
	FACTOR="-f"
fi

IDENTIFIER="${IDENTIFIER}_${MIN_WINDOW}_${MAX_WINDOW}"
OUTPUT_JSON="${S3M_OUTDIR}/${IDENTIFIER}.json"
echo "run ../build/s3m $FACTOR $SQUEEZE $MERGE_TABLE $KEEP_NORMAL_ONLY $DISABLE_PRUNING $REMOVE_DUPLICATES -m $MIN_WINDOW -M $MAX_WINDOW -s $STRIDE -l $LABEL_INDEX -k $KEEP -i $INPUT -o $OUTPUT_JSON"
../build/s3m $FACTOR $SQUEEZE $MERGE_TABLE $KEEP_NORMAL_ONLY $DISABLE_PRUNING $REMOVE_DUPLICATES -m $MIN_WINDOW -M $MAX_WINDOW -s $STRIDE -l $LABEL_INDEX -k $KEEP -i $INPUT -o $OUTPUT_JSON

echo "Evaluating and Visualizing Shapelets"
python3 ShapeletEvaluation.py -tr $INPUT -te $TEST -i $OUTPUT_JSON -o $OUTDIR -n $IDENTIFIER -l $LABEL_INDEX -k $KEEP
