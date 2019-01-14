#include "distances/Minkowski.hh"

#include "TimeSeries.hh"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include <cmath>

MinkowskiDistance::MinkowskiDistance( ValueType p )
  : _p( p )
{
  if( _p < ValueType() )
    throw std::runtime_error( "Power parameter must be nonnegative" );
}

MinkowskiDistance::ValueType MinkowskiDistance::operator()( const TimeSeries& S, const TimeSeries& T )
{
  const TimeSeries* T1 = &S;
  const TimeSeries* T2 = &T;

  if( T1->length() > T2->length() )
    std::swap( T1, T2 );

  ValueType distance = std::numeric_limits<ValueType>::max();

  for( std::size_t i = 0; i <= T2->length() - T1->length(); i++ )
  {
    ValueType temp = ValueType();
    for( std::size_t j = 0; j < T1->length(); j++ )
    {
      auto x = (*T2)[i+j];
      auto y = (*T1)[  j];
      auto d = static_cast<ValueType>(x - y);

      temp += std::pow( d, _p );

      // Abandon early if we are already worse than the current best
      // estimate.
      if( temp > distance )
        break;
    }

    distance = std::min( distance, temp );

    // This is the closest possible distance, so we might as well stop
    // calculating here.
    if( distance == ValueType() )
      return distance;
  }

  return distance;
}
