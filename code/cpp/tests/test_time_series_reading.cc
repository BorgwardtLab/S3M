#include <cassert>

#include "TimeSeries.hh"
#include "Utilities.hh"

int main( int, char** )
{
  // Default settings for reading --------------------------------------

  {
    auto data         = readData( "./data/short.csv" );
    auto&& timeSeries = data.first;

    assert( timeSeries.size() == 4 );
  }
}
