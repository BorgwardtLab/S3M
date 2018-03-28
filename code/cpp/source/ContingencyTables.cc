#include "ContingencyTables.hh"
#include "LookupTable.hh"

#include <algorithm>

#include <cassert>
#include <cmath>

ContingencyTables::ContingencyTables( unsigned n, unsigned n1, LookupTable& lookupTable )
  : _n( n )
  , _n1( n1 )
  , _lookupTable( lookupTable )
{
}

void ContingencyTables::insert( double distance, bool label )
{
  // Search for the table that is supposed to be updated. Tables are
  // kept sorted according to their threshold.
  auto itPosition = std::lower_bound( _tables.begin(), _tables.end(),
    ContingencyTable( _n, _n1, distance ),
    [] ( const ContingencyTable& table1, const ContingencyTable& table2 )
    {
      return table1.threshold() < table2.threshold();
    }
  );

  // Need to insert a new table and update it according to the labels
  // that we have already seen so far.
  if( itPosition == _tables.end() or itPosition->threshold() != distance )
  {
    ContingencyTable table( _n, _n1, distance );

    for( auto&& distanceLabelPair : _distanceLabelPairs )
      table.insert( distanceLabelPair.first, distanceLabelPair.second );

    _tables.insert( itPosition, table );
  }

  // Update *all* tables, including the new one, with the new distance
  // and the new label.
  for( auto&& table : _tables )
    table.insert( distance, label );

  _distanceLabelPairs.emplace_back( std::make_pair( distance, label ) );
}

long double ContingencyTables::min_p() const noexcept
{
  long double p = 1.0;

  for( auto&& table : _tables )
    p = std::min( p, table.p() );

  assert( std::isfinite( p ) );
  return p;
}

long double ContingencyTables::min_optimistic_p() const noexcept
{
  long double p = 1.0;

  for( auto&& table : _tables )
    p = std::min( p, table.min_optimistic_p() );

  assert( std::isfinite( p ) );
  return p;
}

std::pair<long double, ContingencyTable> ContingencyTables::min() const noexcept
{
  assert( not _tables.empty() );

  auto it
    = std::min_element( _tables.begin(), _tables.end(),
        [] ( const ContingencyTable& t1, const ContingencyTable& t2 )
        {
          return t1.p() < t2.p();
        }
      );

  return std::make_pair( it->p(), *it );
}

std::size_t ContingencyTables::prune( long double p )
{
  auto n = this->size();

  _tables.erase(
    std::remove_if( _tables.begin(), _tables.end(),
      [&p, this] ( const ContingencyTable& table )
      {
        return table.min_optimistic_p( _lookupTable ) > p;
      }
    ),
    _tables.end()
  );

  return n - this->size();
}
