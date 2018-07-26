#include "Utilities.hh"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>

#include <cassert>

std::pair< std::vector<TimeSeries>, std::vector<bool> > readData( const std::string& filename, unsigned l, const std::vector<unsigned>& excludeColumns )
{
  std::ifstream in( filename );
  if( !in )
    throw std::runtime_error( "Unable to open input file" );

  using ValueType = typename TimeSeries::ValueType;

  std::vector<TimeSeries> result;
  std::vector<ValueType> rawLabels;

  while( in )
  {
    // Skip comment lines, regardless of their position within the file,
    // as this permits us to describe our data.
    if( in.peek() == '#' )
    {
      std::string line;
      std::getline( in, line );

      continue;
    }

    TimeSeries T;
    in >> T;

    assert( l < T.length() );

    // Extract & remove the label afterwards
    rawLabels.push_back( T[l] );

    // Clean the time series by removing the label and all columns that
    // we want to exclude from the calculation
    std::vector<ValueType> rawValues( T.begin(), T.end() );
    std::vector<ValueType> newValues;
    for( unsigned i = 0; i < static_cast<unsigned>( rawValues.size() ); i++ )
    {
      if( i == l || std::find( excludeColumns.begin(), excludeColumns.end(), i ) != excludeColumns.end() )
        newValues.push_back( rawValues[i] );
    }

    T = TimeSeries( newValues.begin(), newValues.end() );

    result.push_back( T );
  }

  assert( result.size() == rawLabels.size() );

  std::vector<bool> labels;

  {
    std::set<ValueType> distinctLabels( rawLabels.begin(), rawLabels.end() );

    assert( distinctLabels.size() == 2 );

    auto l1 = *distinctLabels.rbegin();

    std::transform( rawLabels.begin(), rawLabels.end(),
                    std::back_inserter( labels ),
                    [&l1] ( const ValueType& label )
                    {
                      return label == l1;
                    }
    );
  }

  return std::make_pair( result, labels );
}

std::pair<double, double> standardizeData( std::vector<TimeSeries>& timeSeries )
{
  using namespace boost::accumulators;
  using ValueType = typename TimeSeries::ValueType;

  accumulator_set<ValueType,
                  features<tag::mean,
                           tag::variance> > accumulator;

  for( auto&& ts : timeSeries )
    for( auto&& value : ts )
      accumulator( value );

  auto mu    = mean( accumulator );
  auto sigma = std::sqrt( variance( accumulator ) );

  for( auto&& ts : timeSeries )
  {
    std::transform( ts.begin(), ts.end(), ts.begin(),
                    [&mu, &sigma] ( const ValueType& value )
                    {
                      return (value - mu) / sigma;
                    }
    );
  }

  return std::make_pair( mu, sigma );
}
