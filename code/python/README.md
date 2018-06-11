The `s3m_eval.sh` script let's you evaluate and visualize the top `k` significant shapelets found by S3M. 

### Minimal Usage Example
    $ ./s3m_eval.sh -i PATH_TO_TRAIN_DATA -e PATH_TO_TEST_DATA
    
### Complete List of Options
    -i  path to training file
    -d  identifier (e.g. Heart_rate)
    -e  path to test file

    -t  Merge equal contingency tables
    -n  Keep only normal p-values
    -p  Disable pruning criterion
    -r  Remove duplicates
    -m  Minimum candidate pattern length (=10)
    -M  Maximum candidate pattern length (=0)
    -s  Stride (=1)
    -l  Index of label in time series (=0)
    -k  Maximum number of shapelets to keep (0 = unlimited)
    -q  Standardize data
    -f  Use default factor and do not apply correction from the paper
