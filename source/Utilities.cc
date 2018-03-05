#include "Utilities.hh"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <fstream>
#include <set>
#include <stdexcept>

#include <cassert>

std::pair< std::vector<TimeSeries>, std::vector<bool> > readData( const std::string& filename, unsigned l )
{
  std::ifstream in( filename );
  if( !in )
    throw std::runtime_error( "Unable to open input file" );

  using ValueType = typename TimeSeries::ValueType;

  std::vector<TimeSeries> result;
  std::vector<ValueType> rawLabels;

  while( in )
  {
    TimeSeries T;
    in >> T;

    assert( l < T.length() );

    // extract & remove the label afterwards
    rawLabels.push_back( T[l] );
    T.pop_front();

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

void standardizeData( std::vector<TimeSeries>& timeSeries )
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
}
