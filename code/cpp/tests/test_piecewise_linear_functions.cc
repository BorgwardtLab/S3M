#include <iostream>

#include <cassert>

#include "PiecewiseLinearFunction.hh"

int main( int, char** )
{
  auto f = PiecewiseLinearFunction( {0.0, 1.0, 0.0} );
  auto g = PiecewiseLinearFunction( {0.0, 0.5, 0.0} );
  auto h = f - f;
}
