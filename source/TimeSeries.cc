#include "TimeSeries.hh"
#include "Utilities.hh"

#include <algorithm>
#include <istream>
#include <limits>
#include <string>

#include <cmath>

bool TimeSeries::operator==( const TimeSeries& other ) const noexcept
{
  if( this->length() != other.length() )
    return false;

  return std::equal( this->begin(), this->end(),
                     other.begin() );
}

bool TimeSeries::operator!=( const TimeSeries& other ) const noexcept
{
  return !this->operator==( other );
}

bool TimeSeries::isClose( const TimeSeries& other, double rtol, double atol ) const noexcept
{
  if( this->length() != other.length() )
    return false;

  for( auto it1 = this->begin(), it2 = other.begin(); it1 != this->end() && it2 != other.end(); ++it1, ++it2 )
  {
    if( std::abs( *it1 - *it2 ) > (atol + rtol * std::abs( *it2 ) ) )
      return false;
  }

  return true;
}

double TimeSeries::distance( const TimeSeries& other ) const noexcept
{
  const TimeSeries* T1 = this;
  const TimeSeries* T2 = &other;

  if( T1->length() > T2->length() )
    std::swap( T1, T2 );

  ValueType distance = std::numeric_limits<ValueType>::max();

  for( std::size_t i = 0; i <= T2->length() - T1->length(); i++ )
  {
    ValueType temp = ValueType();
    for( std::size_t j = 0; j < T1->length(); j++ )
    {
      auto x = (*T2)[i+j];
      auto y = (*T1)[  j];

      temp += (x-y) * (x-y);

      // Abandon early if we are already worse than the current best
      // estimate.
      if( temp > distance )
        break;
    }

    distance = std::min( distance, temp );

    // This is the closest possible distance, so we might as well stop
    // calculating here.
    if( distance == ValueType() )
      return distance;
  }

  return distance;
}

double TimeSeries::distanceCentred( const TimeSeries& other ) const noexcept
{
  const TimeSeries* T1 = this;
  const TimeSeries* T2 = &other;

  if( T1->length() > T2->length() )
    std::swap( T1, T2 );

  ValueType distance = std::numeric_limits<ValueType>::max();

  for( std::size_t i = 0; i <= T2->length() - T1->length(); i++ )
  {
    ValueType temp = ValueType();
    auto offset    = (*T2)[i] - (*T1)[0];
    for( std::size_t j = 0; j < T1->length(); j++ )
    {
      auto x = (*T2)[i+j];
      auto y = (*T1)[  j];

      temp += (x-y-offset) * (x-y-offset);

      // Abandon early if we are already worse than the current best
      // estimate.
      if( temp > distance )
        break;
    }

    distance = std::min( distance, temp );

    // This is the closest possible distance, so we might as well stop
    // calculating here.
    if( distance == ValueType() )
      return distance;
  }

  return distance;
}

std::istream& operator>>( std::istream& in, TimeSeries& T )
{
  using ValueType = typename TimeSeries::ValueType;

  std::string line;
  std::getline( in, line );

  std::vector<ValueType> values;

  auto tokens = split( line, std::string( "[:;,[:space:]]+" ) );
  for( auto&& token : tokens )
  {
    auto value = convert<ValueType>( token );
    values.emplace_back( value );
  }

  T = TimeSeries( values.begin(), values.end() );

  // FIXME: this is a quite hackish solution; it should be solvable by
  // direct queries of the `istream` object
  if( in.peek() == std::char_traits<char>::eof() )
    in.setstate( std::ios_base::eofbit | std::ios_base::failbit );

  return in;
}

std::ostream& operator<<( std::ostream& out, const TimeSeries& T )
{
  for( std::size_t i = 0; i < T.length(); i++ )
  {
    if( i > 0 )
      out << " ";
    out << T[i];
  }

  return out;
}
