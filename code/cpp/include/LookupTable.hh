#ifndef LOOKUP_TABLE_HH__
#define LOOKUP_TABLE_HH__

#include <vector>

#include <boost/math/distributions/chi_squared.hpp>

class LookupTable
{
public:
  LookupTable( unsigned n, unsigned n1 );

  /**
    Calculates or returns the minimum attainable $p$-value of a given
    marginal value.
  */

  long double operator[]( unsigned rs );

private:
  unsigned _n  = 0;
  unsigned _n1 = 0;

  /** Shared distribution for calculating $p$-values */
  static boost::math::chi_squared_distribution<long double> _chi2;

  /** Maps marginals to $p$-values */
  std::vector<long double> _values;
};

#endif
