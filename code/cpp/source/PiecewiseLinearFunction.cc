#include "PiecewiseLinearFunction.hh"

#include <functional>

PiecewiseLinearFunction PiecewiseLinearFunction::operator+( const PiecewiseLinearFunction& other ) const
{
  return this->apply( other, std::plus<ValueType>() );
}

PiecewiseLinearFunction PiecewiseLinearFunction::operator-( const PiecewiseLinearFunction& other ) const
{
  return this->apply( other, std::minus<ValueType>() );
}

//  def norm(self, p=2.0):
//      result = 0.0
//      for first, second in zip(zip(self.x, self.y), zip(self.x[1:], self.y[1:])):
//          x1, y1 = first[0] , first[1]
//          x2, y2 = second[0], second[1]
//          m      = (y2 - y1) / (x2 - x1)
//          c      = y1 - m * x1
//
//          def evaluator(x):
//              if m == 0.0:
//                  return math.pow(c,p) * x
//              else:
//                  return math.pow( m*x + c, p+1 ) / ( m * (p+1) );
//          
//          integral  = abs( evaluator(x2) - evaluator(x1) )
//          result   += integral
//
//      return math.pow(result, 1.0 / p)

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
