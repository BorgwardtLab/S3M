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

    std::cerr << timeSeries.size() << "\n";

    assert( timeSeries.size() == 4 );
  }
}
