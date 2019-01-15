#include "distances/Lp.hh"

#include "PiecewiseLinearFunction.hh"

#include <utility>

#include <cassert>
#include <cmath>

LpDistance::LpDistance( ValueType p )
  : _p( p )
{
}

LpDistance::ValueType LpDistance::operator()( const TimeSeries& S, const TimeSeries& T ) const
{
  const TimeSeries* shapelet   = &S;
  const TimeSeries* timeSeries = &T;

  if( shapelet->length() > timeSeries->length() )
    std::swap( shapelet, timeSeries );

  auto n = shapelet->length();
  auto m = timeSeries->length();

  assert( n <= m );

  auto f      = PiecewiseLinearFunction( shapelet->begin(), shapelet->end() );
  ValueType d = std::numeric_limits<ValueType>::max();

  // For each offset, construct a time series of the correct length and
  // compare it against the shapelet.
  for( std::size_t i = 0; i < m - n + 1; i++ )
  {
    std::vector<ValueType> values( n );

    std::copy( timeSeries->begin() + static_cast<long>(i),
               timeSeries->begin() + static_cast<long>(i + n),
               values.begin() );

    assert( values.size() == n );

    auto g = PiecewiseLinearFunction( values.begin(), values.end() );
    d = std::min( d, std::abs( (f - g).norm( _p ) ) );
  }

  return d;
}
