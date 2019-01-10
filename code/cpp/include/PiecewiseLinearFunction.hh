#ifndef PIECEWISE_LINEAR_FUNCTION_HH__
#define PIECEWISE_LINEAR_FUNCTION_HH__

// Require this only to get the same value type as the time series such
// that subsequent changes to this container do not affect us.
#include "TimeSeries.hh"

#include <initializer_list>

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

  // Attributes --------------------------------------------------------

  /** Returns current size or length of time series */
  std::size_t size()   const noexcept { return _values.size(); }
  std::size_t length() const noexcept { return _values.size(); }

  /** Checks whether the current piecewise linear function is empty */
  bool empty() const noexcept { return _values.empty(); }

private:

  bool hasSameDomain( const PiecewiseLinearFunction& other ) const;
  bool hasSameRange( const PiecewiseLinearFunction& other ) const;

  ContainerType _values;
};

#endif
