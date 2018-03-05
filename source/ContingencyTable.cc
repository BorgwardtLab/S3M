#include "ContingencyTable.hh"

#include <boost/math/distributions/hypergeometric.hpp>
#include <boost/math/special_functions/binomial.hpp>

#include <algorithm>
#include <limits>
#include <numeric>

#include <cassert>
#include <cmath>

// Shared probability distribution for all contingency tables. It makes
// no sense to provide multiple copies here.
boost::math::chi_squared_distribution<long double> ContingencyTable::_chi2
  = boost::math::chi_squared_distribution<long double>( 1 );

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

unsigned ContingencyTable::n1() const noexcept
{
  // TODO: use memoization to speed up spurious lookups?
  return _as + _bs;
}

unsigned ContingencyTable::n0() const noexcept
{
  // TODO: use memoization to speed up spurious lookups?
  return _cs + _ds;
}

unsigned ContingencyTable::rs() const noexcept
{
  // TODO: use memoization to speed up spurious lookups?
  return _as + _ds;
}

unsigned ContingencyTable::qs() const noexcept
{
  // TODO: use memoization to speed up spurious lookups?
  return _bs + _cs;
}

long double ContingencyTable::p() const
{
  // TODO: use memoization to speed up spurious lookups?
  return static_cast<long double>( 1.0 ) - boost::math::cdf( _chi2, this->t() );
}

long double ContingencyTable::p_raw() const
{
  // TODO: use memoization to speed up spurious lookups?
  return static_cast<long double>( 1.0 ) - boost::math::cdf( _chi2, this->t_raw() );
}

long double ContingencyTable::min_attainable_p() const
{
  return this->min_attainable_p( this->rs() );
}

long double ContingencyTable::min_attainable_p( unsigned rs ) const
{
  auto na = std::min(_n1, _n - _n1);
  auto nb = std::max(_n1, _n - _n1);

  assert( rs <= _n );

  long double x = 0.0;

  if( rs < na )
    x = (_n-1) * nb/static_cast<long double>(na) * rs/static_cast<long double>(_n - rs);
  else if( na <= rs and rs < _n/2 )
    x = (_n-1) * na/static_cast<long double>(nb) * (_n - rs)/static_cast<long double>(rs);
  else if( _n/2 <= rs and rs < nb )
    x = (_n-1) * na/static_cast<long double>(nb) * rs/static_cast<long double>(_n - rs);
  else if( nb <= rs and rs <= _n )
    x = (_n-1) * nb/static_cast<long double>(na) * (_n - rs)/static_cast<long double>(rs);

  return static_cast<long double>(1.0) - boost::math::cdf( _chi2, x );
}

long double ContingencyTable::min_attainable_partial_p() const
{
  // TODO:
  //  - not sure whether this function could be merged into the regular
  //  function for attainable $p$-values
  //
  //  - check for memoization strategies; it should be possible to save
  //  a lot of spurious calculations here

  long double p = 1.0;
  auto rs       = this->rs();
  auto qs       = this->qs();
  auto delta    = _n - (rs+qs);

  for( unsigned i = 0; i <= delta; i++ )
    p = std::min( p, this->min_attainable_p( rs+i ) );

  return p;
}

long double ContingencyTable::min_optimistic_p() const
{
  auto n1 = this->n1(); // marginals (first row)
  auto n0 = this->n0(); // marginals (second row)
  auto m1 = _n1 - n1;   // missing objects (y=1)
  auto m0 = _n0 - n0;   // missing objects (y=0)

  assert( m1 < _n1 );
  assert( m0 < _n0 );

  // TODO: check for spurious calls, for example when m1 == m0 == 0 and
  // no optimistic prognosis can be made any more...

  // Case 1: all remaining instances are classified correctly

  auto C1  = *this;
  C1._as  += m1;
  C1._cs  += m0;

  // Case 2: all remaining instances are classified incorrectly

  auto C2  = *this;
  C2._bs  += m1;
  C2._ds  += m0;

  return std::min( C1.p(), C2.p() );
}

long double ContingencyTable::min_optimistic_p( unsigned delta ) const
{
  assert( this->n1() == _n1 );
  assert( this->n0() == _n0 );

  // Assign only half of the objects into the cells of the table in
  // order not to break symmetry.
  assert( delta % 2 == 0 );
  delta = delta / 2;

  long double p = 1.0;

  if( _as >= delta and _cs >= delta and _ds + delta <= _n0 and _bs + delta <= _n1 )
  {
    auto C  = *this;
    C._as  -= delta;
    C._bs  += delta;
    C._cs  -= delta;
    C._ds  += delta;

    p = C.p();
  }

  if( _as + delta <= _n1 and _cs + delta <= _n0 and _ds >= delta and _bs >= delta )
  {
    auto C  = *this;
    C._as  += delta;
    C._bs  -= delta;
    C._cs  += delta;
    C._ds  -= delta;

    p = std::min( p, C.p() );
  }

  return p;
}

double ContingencyTable::p_fisher() const
{
  // TODO: this distribution should not be created *every* time the
  // function is called, but its shape depends on `rs`...
  boost::math::hypergeometric_distribution<double> distribution( this->rs(), _n1, _n );

  // This is only the probability of the contingency table itself. The
  // count variable needs to be adjusted to obtain a two-tailed value.
  auto prob_initial = boost::math::pdf( distribution, _as );

  std::vector<double> probabilities;
  probabilities.push_back( prob_initial );

  auto as_min = _n1 + this->rs() > _n ? _n1 + this->rs() - _n : 0;
  auto as_max = this->rs() > _n1      ? _n1                   : this->rs();

  for( unsigned as = as_min; as <= as_max; as++ )
  {
    if( as == _as )
      continue;

    auto prob = boost::math::pdf( distribution, as );
    if( prob <= prob_initial )
      probabilities.push_back( prob );
  }

  return std::accumulate( probabilities.begin(), probabilities.end(), 0.0 );
}

double ContingencyTable::min_attainable_p_fisher( unsigned rs ) const
{
  auto na = std::min(_n1, _n - _n1);
  auto nb = std::max(_n1, _n - _n1);

  assert( rs <= _n );

  double x           = 0.0;
  double denominator = boost::math::binomial_coefficient<double>( _n, rs );

  if( rs < na )
    x = boost::math::binomial_coefficient<double>( na, rs );
  else if( na <= rs and rs < _n/2 )
    x = boost::math::binomial_coefficient<double>( nb, _n - rs );
  else if( _n/2 <= rs and rs < nb )
    x = boost::math::binomial_coefficient<double>( nb, rs );
  else if( nb <= rs and rs <= _n )
    x = boost::math::binomial_coefficient<double>( na, _n - rs );

  return x / denominator;
}

bool ContingencyTable::complete() const noexcept
{
  // TODO: use memoization to speed up spurious lookups?
  return _as + _bs + _cs + _ds == _n;
}

double ContingencyTable::t() const
{
  // TODO: use memoization to speed up spurious lookups?

  auto numerator   = _n * std::pow( double(_as*_cs) - double(_bs*_ds), 2.0 );
  auto denominator = (_as+_bs) * (_cs+_ds) * (_as+_ds) * (_bs+_cs);

  return static_cast<double>( numerator / denominator );
}

double ContingencyTable::t_raw() const
{
  // TODO: use memoization to speed up spurious lookups?

  // Use the *uncorrected* values. This may be an extremely stupid idea
  // by the client, but who are we to discourage them?
  auto as = this->as();
  auto bs = this->bs();
  auto cs = this->cs();
  auto ds = this->ds();
  auto n  = _n - 4;

  auto numerator   = n * std::pow( double(as*cs) - double(bs*ds), 2.0 );
  auto denominator = (as+bs) * (cs+ds) * (as+ds) * (bs+cs);

  return static_cast<double>( numerator / denominator );
}

std::ostream& operator<<( std::ostream& o, const ContingencyTable& C )
{
  o << C.as() << "," << C.bs() << "," << C.ds() << "," << C.cs();
  return o;
}
