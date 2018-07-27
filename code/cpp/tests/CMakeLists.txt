ADD_EXECUTABLE( test_time_series_reading
  test_time_series_reading.cc
  #
  ../source/TimeSeries.cc
  ../source/Utilities.cc
)

ADD_TEST( TimeSeriesReading test_time_series_reading )
