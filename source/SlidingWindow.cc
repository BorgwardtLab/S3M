#include "SlidingWindow.hh"
#include "TimeSeries.hh"

#include <algorithm>

#include <cassert>

SlidingWindow::SlidingWindow( unsigned size, unsigned stride )
  : _minSize( size )
  , _maxSize( size )
  , _stride( stride )
{
  assert( _minSize >= 1 );
  assert( _stride  >= 1 );
}

SlidingWindow::SlidingWindow( unsigned minSize, unsigned maxSize, unsigned stride )
  : _minSize( minSize )
  , _maxSize( maxSize )
  , _stride( stride )
{
  assert( _minSize >= 1 );
  assert( _maxSize >= 1 );
  assert( _stride  >= 1 );
  assert( _minSize <= maxSize );
}

void SlidingWindow::setRemoveDuplicates( bool value )
{
  _removeDuplicates = value;
}

bool SlidingWindow::removeDuplicates() const noexcept
{
  return _removeDuplicates;
}

std::vector<TimeSeries> SlidingWindow::operator()( const TimeSeries& T ) const noexcept
{
  std::vector<TimeSeries> candidates;

  // Required to silence some warnings about the candidate generation
  // code below.
  using DifferenceType = typename std::vector<TimeSeries>::difference_type;
  auto n               = T.length();

  for( unsigned size = _minSize; size <= _maxSize; size++ )
  {
    for( decltype(n) i = 0; i < n; i += _stride )
    {
      // No valid pattern
      if( i + size > T.length() )
        break;

      TimeSeries candidate
        = TimeSeries( T.begin() + DifferenceType(i),
                      T.begin() + DifferenceType(i + size ) );

      // Create a new predicate for closeness detection. If any other
      // time series satisfies this predicate, the new candidate will
      // not be added.
      //
      // TODO: make the parameters configurable here
      auto isClose = [&candidate] ( const TimeSeries& t )
      {
        return t.isClose( candidate );
      };

      // Perform a check for duplicate time series. Notice that this
      // does not use *rounding* but full equality comparisons.
      if( not _removeDuplicates or std::none_of( candidates.begin(), candidates.end(), isClose ) )
      {
        candidates.emplace_back( candidate );

        assert( candidates.back().length() == size );
      }
    }
  }

  return candidates;
}
