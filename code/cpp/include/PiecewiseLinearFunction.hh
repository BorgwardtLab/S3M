#ifndef PIECEWISE_LINEAR_FUNCTION_HH__
#define PIECEWISE_LINEAR_FUNCTION_HH__

// Require this only to get the same value type as the time series such
// that subsequent changes to this container do not affect us.
#include "TimeSeries.hh"

#include <initializer_list>
#include <stdexcept>

class PiecewiseLinearFunction
{
public:
  using ValueType      = TimeSeries::ValueType;
  using ContainerType  = TimeSeries::ContainerType;
  using const_iterator = typename ContainerType::const_iterator;
  using iterator       = typename ContainerType::iterator;

  // Constructors ------------------------------------------------------

  template <class InputIterator> PiecewiseLinearFunction( InputIterator begin, InputIterator end)
    : _values( begin, end )
  {
  }

  PiecewiseLinearFunction( std::initializer_list<ValueType>&& il )
    : _values( il.begin(), il.end() )
  {
  }

  // Iterators ---------------------------------------------------------

  const_iterator begin() const noexcept { return _values.begin(); }
  iterator       begin()       noexcept { return _values.begin(); }

  const_iterator end()   const noexcept { return _values.end(); }
  iterator       end()         noexcept { return _values.end(); }

  // Attributes --------------------------------------------------------

  /** Returns current size or length of time series */
  std::size_t size()   const noexcept { return _values.size(); }
  std::size_t length() const noexcept { return _values.size(); }

  /** Checks whether the current piecewise linear function is empty */
  bool empty() const noexcept { return _values.empty(); }

  // Operators ---------------------------------------------------------

  PiecewiseLinearFunction operator+( const PiecewiseLinearFunction& other ) const;
  PiecewiseLinearFunction operator-( const PiecewiseLinearFunction& other ) const;

private:

  template <class BinaryOperation> PiecewiseLinearFunction apply(
    const PiecewiseLinearFunction& other,
    BinaryOperation operation ) const
  {
    if( !this->hasSameDomain( other ) || !this->hasSameRange( other ) )
      throw std::runtime_error( "Domain and range of piecewise linear functions must agree" );

    ContainerType values;
    values.reserve( this->length() );

    for( auto it1 = this->begin(), it2 = other.begin(); it1 != this->end() && it2 != other.end(); ++it1, ++it2 )
    {
      values.push_back( operation( *it1, *it2 ) );
    }

    return PiecewiseLinearFunction( values.begin(), values.end() );
  }

  bool hasSameDomain( const PiecewiseLinearFunction& other ) const;
  bool hasSameRange( const PiecewiseLinearFunction& other ) const;

  ContainerType _values;
};

#endif
