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
