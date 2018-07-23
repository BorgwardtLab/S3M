#include "ContingencyTable.hh"
#include "LookupTable.hh"

#include <algorithm>
#include <limits>
#include <numeric>

#include <cassert>
#include <cmath>

// Shared probability distribution for all contingency tables. It makes
// no sense to provide multiple copies here.
boost::math::chi_squared_distribution<long double> ContingencyTable::_chi2
  = boost::math::chi_squared_distribution<long double>( 1 );

LookupTable ContingencyTable::_lookupTable
  = LookupTable( 0, 0);

ContingencyTable::ContingencyTable()
  : _n ( 0 )
  , _n1( 0 )
  , _n0( 0 )
  , _as( 0 )
  , _bs( 0 )
  , _cs( 0 )
  , _ds( 0 )
  , _threshold( std::numeric_limits<double>::quiet_NaN() )
{
}

ContingencyTable::ContingencyTable( unsigned n, unsigned n1, double threshold )
  : _n ( n  + 4   )
  , _n1( n1 + 2   )
  , _n0( _n - _n1 )
  , _as( 1 )
  , _bs( 1 )
  , _cs( 1 )
  , _ds( 1 )
  , _threshold( threshold )
{
  assert( _n >= _n1 );
  assert( _n >= _n0 );
  assert( _n == _n0 + _n1 );

  if( _lookupTable.n() != _n && _lookupTable.n1() != _n1 )
    _lookupTable = LookupTable( _n, _n1 );
}

void ContingencyTable::insert( double distance, bool label )
{
  // Left column (as or ds)
  if( distance <= _threshold )
  {
    // Upper row (as)
    if( label )
      _as += 1;
    else
      _ds += 1;
  }

  // Right column (bs or cs)
  else
  {
    // Upper row (bs)
    if( label )
      _bs += 1;
    else
      _cs += 1;
  }

  assert( _as + _bs <= _n1 );
  assert( _ds + _cs <= _n0 );
}

bool ContingencyTable::operator==( const ContingencyTable& other ) const noexcept
{
  return   _as == other._as
        && _bs == other._bs
        && _cs == other._cs
        && _ds == other._ds
        && _n1 == other._n1
        && _n0 == other._n0
        && _n  == other._n; // this last check is somewhat superfluous,
                            // but what the heck, let's do it anyway
}

unsigned ContingencyTable::n1() const noexcept
{
  return _as + _bs;
}

unsigned ContingencyTable::n0() const noexcept
{
  return _cs + _ds;
}

unsigned ContingencyTable::rs() const noexcept
{
  return _as + _ds;
}

unsigned ContingencyTable::qs() const noexcept
{
  return _bs + _cs;
}

long double ContingencyTable::p() const
{
  bool withPseudocounts = true;
  return boost::math::cdf( boost::math::complement( _chi2, this->t( withPseudocounts ) ) );
}

long double ContingencyTable::p_raw() const
{
  bool withPseudocounts = false;
  return boost::math::cdf( boost::math::complement( _chi2, this->t( withPseudocounts ) ) );
}

long double ContingencyTable::min_attainable_p() const
{
  return this->min_attainable_p( this->rs() );
}

long double ContingencyTable::min_attainable_p( unsigned rs ) const
{
  return _lookupTable[ rs ];
}

long double ContingencyTable::min_optimistic_p() const
{
  auto n1 = this->n1(); // marginals (first row)
  auto n0 = this->n0(); // marginals (second row)
  auto m1 = _n1 - n1;   // missing objects (y=1)
  auto m0 = _n0 - n0;   // missing objects (y=0)

  assert( m1 < _n1 );
  assert( m0 < _n0 );

  return std::min(
    std::min( _lookupTable[ this->rs() + m1 ], _lookupTable[ this->rs() + m0 ] ),
    _lookupTable[ this->rs() ]
  );
}

bool ContingencyTable::complete() const noexcept
{
  return _as + _bs + _cs + _ds == _n;
}

long double ContingencyTable::t( bool withPseudocounts ) const
{
  long double as = withPseudocounts ? _as : this->as();
  long double bs = withPseudocounts ? _bs : this->bs();
  long double cs = withPseudocounts ? _cs : this->cs();
  long double ds = withPseudocounts ? _ds : this->ds();
  long double n  = as+bs+cs+ds;
  long double n1 = as+bs;
  long double n0 = cs+ds;
  long double rs = as+ds;
  long double qs = bs+cs;

  // Expected frequencies, lovingly calculated and checked manually;
  // note that we give an explicit type in order to ensure that each
  // term will have the proper type
  long double ea = (n1*rs) / n;
  long double eb = (n1*qs) / n;
  long double ec = (n0*qs) / n;
  long double ed = (n0*rs) / n;

  long double t = ((as-ea)*(as-ea))/ea
                + ((bs-eb)*(bs-eb))/eb
                + ((cs-ec)*(cs-ec))/ec
                + ((ds-ed)*(ds-ed))/ed;

  return t;
}

long double ContingencyTable::t_raw() const
{
  // Use the *uncorrected* values. This may be an extremely dangerous
  // idea by the client in case the table is not valid.
  auto as = this->as();
  auto bs = this->bs();
  auto cs = this->cs();
  auto ds = this->ds();
  auto n  = _n - 4;

  auto numerator   = n * std::pow( static_cast<long double>(as*cs) - static_cast<long double>(bs*ds), static_cast<long double>( 2.0 ) );
  auto denominator = (as+bs) * (cs+ds) * (as+ds) * (bs+cs);

  return static_cast<double>( numerator / denominator );
}

std::ostream& operator<<( std::ostream& o, const ContingencyTable& C )
{
  o << C.as() << "," << C.bs() << "," << C.ds() << "," << C.cs();
  return o;
}
