#include <iostream>

#include <cassert>

#include "PiecewiseLinearFunction.hh"

int main( int, char** )
{
  auto f = PiecewiseLinearFunction( {0.0, 1.0, 0.0} );
  auto g = PiecewiseLinearFunction( {0.0, 0.5, 0.0} );
  auto h = f - f;

  assert( f.length() == 3);
  assert( f.length() == g.length() );
  assert( f.length() == h.length() );

  assert( f.norm(1) == 1.00 );
  assert( g.norm(1) == 0.50 );
  assert( h.norm(1) == 0.00 );
}
