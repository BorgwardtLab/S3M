#include "LookupTable.hh"

#include <algorithm>

// Shared probability distribution for all lookup tables. It makes no
// sense to provide multiple copies here.
boost::math::chi_squared_distribution<long double> LookupTable::_chi2
  = boost::math::chi_squared_distribution<long double>( 1 );

LookupTable::LookupTable( unsigned n, unsigned n1 )
  : _n ( n  )
  , _n1( n1 )
  , _values( n )
{
  unsigned rs = 0;

  std::transform( _values.begin(), _values.end(), _values.begin(),
    [&rs, this] ( long double /* value */ )
    {
      auto na = std::min(_n1, _n - _n1);
      auto nb = std::max(_n1, _n - _n1);

      long double x = 0.0;

      if( rs < na )
        x = (_n-1) * nb/static_cast<long double>(na) * rs/static_cast<long double>(_n - rs);
      else if( na <= rs and rs < _n/2 )
        x = (_n-1) * na/static_cast<long double>(nb) * (_n - rs)/static_cast<long double>(rs);
      else if( _n/2 <= rs and rs < nb )
        x = (_n-1) * na/static_cast<long double>(nb) * rs/static_cast<long double>(_n - rs);
      else if( nb <= rs and rs <= _n )
        x = (_n-1) * nb/static_cast<long double>(na) * (_n - rs)/static_cast<long double>(rs);

      ++rs;
      return static_cast<long double>(1.0) - boost::math::cdf( _chi2, x );
    }
  );
}
