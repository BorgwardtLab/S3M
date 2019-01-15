#include "PiecewiseLinearFunction.hh"

#include <functional>
#include <iterator>

#include <cmath>
#include <cassert>

PiecewiseLinearFunction PiecewiseLinearFunction::operator+( const PiecewiseLinearFunction& other ) const
{
  return this->apply( other, std::plus<ValueType>() );
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-( const PiecewiseLinearFunction& other ) const
{
  return this->apply( other, std::minus<ValueType>() );
}

PiecewiseLinearFunction::ValueType PiecewiseLinearFunction::norm( double p ) const
{
  ValueType result = ValueType();

  for( auto it = this->begin(); it != this->end(); ++it )
  {
    auto next = std::next( it );
    if( next == this->end() )
      break;

    auto xi = std::distance( this->begin(), it );
    auto yi = *it;
    auto xj = std::distance( this->begin(), next ); // should be xi + 1
    auto yj = *next;

    // These two expressions are guaranteed to be valid for valid
    // iterators.
    auto m = (yj - yi) / (xj - xi);
    auto c = yi - m * xi;

    auto evaluator = [&]( ValueType x )
    {
      if( m == ValueType() )
        return std::pow( c, static_cast<ValueType>(p) ) * x;
      else
        return std::pow( m * x + c, static_cast<ValueType>(p + 1) ) / ( m * (p + 1) );
    };

    assert( p >= 0 );

    // The client wants to calculate the usual $p$-norm, so we do not
    // have to worry about anything else here.
    if( p > 0 )
    {
      auto integral = std::abs( evaluator(xj) - evaluator(xi) );
      result += integral;
    }

    // The client wants to calculate the supremum norm, so we just take
    // the *largest* integral interval that we can find.
    else
      result = std::max( result, std::max( yi, yj ) );
  }

  return result;
}

bool PiecewiseLinearFunction::hasSameDomain( const PiecewiseLinearFunction& ) const
{
  // Since we do not include the domain in our calculation of the
  // piecewise linear functions, this operation is true for now.
  return true;
}

bool PiecewiseLinearFunction::hasSameRange( const PiecewiseLinearFunction& other ) const
{
  return this->length() == other.length();
}
