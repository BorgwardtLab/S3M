# Data sets

This folder contains the data sets that we used to obtain the results
reported in the paper. All data have been obtained from the [MIMIC
Critical Care Database](https://mimic.physionet.org); please refer to
the paper for more details.

Folders:
 
- `training`: contains the files we used for training our method, i.e.,
  the data that we used for extracting statistically significant
  shapelets in the first place
- `test`: contains the files we used for testing the results of our
  method in the context of *classifying* time series

In the following, we will describe some operations that you can apply to
the data for your application.

## Subsampling

The *blood pressure* data has been downsampled because of the large
number of very similar time series with respect to the cases and
controls. The `training` folder contains the sample that we used to
obtain the results in the paper. Please follow these steps in your
preferred terminal emulator to create your own subsampled version of the
data&nbsp;(assuming that you have access to a POSIX-like operating
system such as Linux or MacOS X):

    ```bash
    # First, we need to extract cases and controls from the input data,
    # so we have to take a look at the first entry of each row.
    $ grep    "^1" Blood_pressure.csv > Blood_pressure_cases.csv
    $ grep - v "^1" Blood_pressure.csv > Blood_pressure_controls.csv
    # We then bring all cases into random order, take 50 of them, which
    # you can of course change, and output them to a new file. Ditto for
    # the controls, which are appended to the new file.
    $ sort -R Blood_pressure_cases.csv    | head -n 50 >  Blood_pressure_random_sample.csv
    $ sort -R Blood_pressure_controls.csv | head -n 50 >> Blood_pressure_random_sample.csv
    # Last, as a good data science tactic, we shuffle the resulting data
    # set in order to ensure that the order in which we specified series
    # does not matter. This is not relevant for our method, but may turn
    # out to be relevant for comparing with other methods.
    $ gshuf -o Blood_pressure_random_sample.csv < Blood_pressure_random_sample.csv
    ```
 
