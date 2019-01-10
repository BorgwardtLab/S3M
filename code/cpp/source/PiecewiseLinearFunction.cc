#include "PiecewiseLinearFunction.hh"

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
