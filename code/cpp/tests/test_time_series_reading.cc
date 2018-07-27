#include <iostream>

#include <cassert>

#include "Environment.hh"
#include "TimeSeries.hh"
#include "Utilities.hh"

int main( int, char** )
{
  // Default settings for reading --------------------------------------

  {
    auto data         = readData( CMAKE_SOURCE_DIR + std::string("/tests/data/short.csv") );
    auto&& timeSeries = data.first;
    auto&& labels     = data.second;

    // Just check the size here and a few values of the time series
    // afterwards to ensure that nothing strange happened.
    assert( timeSeries.size() == 4 );
    assert( timeSeries[0].length() ==  5 );
    assert( timeSeries[1].length() ==  6 );
    assert( timeSeries[2].length() == 10 );
    assert( timeSeries[3].length() == 10 );

    assert( timeSeries[0][0] ==   1.0 );
    assert( timeSeries[0][4] ==   7.0 );
    assert( timeSeries[1][0] == 423.0 );
    assert( timeSeries[1][5] ==   0.0 );
    assert( timeSeries[2][0] ==   2.0 );
    assert( timeSeries[2][9] ==  21.0 );
    assert( timeSeries[3][0] ==   2.0 );
    assert( timeSeries[3][9] ==  20.0 );

    // Check that labels can be read correctly
    assert( labels.size() == 4 );
    assert( labels[0] == false );
    assert( labels[1] == true );
    assert( labels[2] == false );
    assert( labels[3] == true );
  }

  // Skipping some columns ---------------------------------------------

  {
    auto data         = readData( CMAKE_SOURCE_DIR + std::string("/tests/data/short.csv"), 0, {6, 8} );
    auto&& timeSeries = data.first;

    assert( timeSeries.size() == 4 );
    assert( timeSeries[0].length() == 5 );
    assert( timeSeries[1].length() == 5 );
    assert( timeSeries[2].length() == 8 );
    assert( timeSeries[3].length() == 8 );

    // Check that shorter time series remain untouched.
    assert( timeSeries[0][0] == 1.0 );
    assert( timeSeries[0][4] == 7.0 );

    // Check that the columns have been successfully removed from longer
    // time series
    assert( timeSeries[2][5] == 15 );
    assert( timeSeries[2][6] == 19 );
    assert( timeSeries[2][7] == 21 );
  }
}
