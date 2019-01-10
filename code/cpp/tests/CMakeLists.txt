ADD_EXECUTABLE( test_time_series_reading
  test_time_series_reading.cc
  #
  ../source/TimeSeries.cc
  ../source/Utilities.cc
)

ADD_TEST( TimeSeriesReading test_time_series_reading )

ADD_EXECUTABLE( test_piecewise_linear_functions
  test_piecewise_linear_functions.cc
  #
  ../source/PiecewiseLinearFunction.cc
)

ADD_TEST( PiecewiseLinearFunctions test_piecewise_linear_functions )
