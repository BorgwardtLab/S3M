#ifndef LOOKUP_TABLE_HH__
#define LOOKUP_TABLE_HH__

#include <vector>

#include <boost/math/distributions/chi_squared.hpp>

class LookupTable
{
public:

  /** Creates a new lookup table for the given marginals */
  LookupTable( unsigned n, unsigned n1 );

  /**
     Returns the minimum attainable $p$-value of a given marginal
     value.
  */

  long double operator[]( unsigned rs ) const noexcept
  {
    return _values[rs];
  }

  // Attributes --------------------------------------------------------

  unsigned n()  const noexcept { return _n;  }
  unsigned n1() const noexcept { return _n1; }

private:
  unsigned _n  = 0;
  unsigned _n1 = 0;

  /** Shared distribution for calculating $p$-values */
  static boost::math::chi_squared_distribution<long double> _chi2;

  /** Maps marginals to $p$-values */
  std::vector<long double> _values;
};

#endif
