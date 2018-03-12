# Results

This folder contains the results that we reported in the paper. Each
data set is stored in [JSON format](https://www.json.org), making it 
easy to perform post-processing with Python.

Here is an excerpt of such a data set:

```json
{
  "parameters": {
    "min_length": 4,
    "max_length": 4,
    "stride": 1,
    "standardize": false,
    "p_tarone": 5.3069488891279228970354608918569e-10,
    "version": "7739eff"
  },
  "shapelets": [
  {
    "p_val": 3.3254540614258143449433191563003e-14,
    "threshold": 203,
    "table": [59,16,12,63],
    "index": 57,
    "start": 16,
    "shapelet": [
      89,105,122,106    ]
  } ]
}
```

Each document starts with an associative array of parameters. The
variables have the following meaning:

* `min_length`: The minimum length of shapelets for the extraction
  process, i.e. the minimum window size.
* `max_length`: The maximum length of shapelets for the extraction
  process, i.e. the maximum window size.
* `stride`: The stride for the shapelet extraction process. This permits
  you to extract shapelets based on *sliding windows* or *tumbling
  windows*.
* `standardize`: Flag indicating whether data have been standardized.
  This is only relevant when you set the appropriate flag during
  shapelet extraction.
* `p_tarone`: The p-value that was reached following Tarone's method for
  correcting for multiple hypothesis testing.
* `version`: A textual identifier referring to the `SHA-1` commit ID
  that was used to obtain the current result. This is highly useful for
  the reproduction of certain results. Simply use `git checkout`, with
  the proper commit ID of course, and build the `s3m` executable.

The individual shapelets are reported as elements of the `shapelets`
list. Again, each shapelet has a set of attributes:

*  `p_val`: The p-value of the shapelet, according to a standard Chi
   square test.
* `threshold`: The distance threshold, specified as a *squared*&nbsp;(!)
  Euclidean distance, that was used to obtain the best contingency table
  with respect to the test statistic. You can use this value to classify
  new time series.
* `table`: The contingency table corresponding to the threshold. The
  values are reported in the order `a,b,d,c`, following the notation
  in the paper.
* `index`: The index of the time series from which the shapelet was
  extracted. This is only used to provide additional details. A value of
  `0` indicates that the shapelet was extracted from the very first time
  series of the data set, for example.
* `start`: The position at which the shapelet was extracted. A start
  value of `0` indicates that the shapelet was taken from the very
  beginning of the time series, for example.
* `shapelet`: A list of y values that make up the shapelet. Notice that
  `index` and `start` can be used to fully reconstruct the position of
  the shapelet with respect to the time series it originated from.
