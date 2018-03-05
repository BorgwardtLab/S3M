#ifndef TIME_SERIES_HH__
#define TIME_SERIES_HH__

#include <initializer_list>
#include <iosfwd>
#include <vector>

/*
  Represents a time series, i.e. a sequence of values of some length,
  for which this class provides lookup and loading functions.
*/

class TimeSeries
{
public:

  // The data type that is used to represent individual values of the
  // time series. Leaving this fixed for now to save some time.
  //
  // TODO: make this a template parameter?
  using ValueType = double;

  using ContainerType  = std::vector<ValueType>;
  using const_iterator = typename ContainerType::const_iterator;
  using iterator       = typename ContainerType::iterator;

  // Makes it possible to create an empty time series and fill it with
  // data later on. The time series will still be valid though, but no
  // extraction algorithm will be able to process them.
  TimeSeries() = default;

  /**
    Creates a new time series from a generic sequence of values. The
    data types are assumed to be compatible.
  */

  template <class InputIterator> TimeSeries( InputIterator begin, InputIterator end )
    : _values( begin, end )
  {
  }

  TimeSeries( std::initializer_list<ValueType>&& il )
    : _values( il.begin(), il.end() )
  {
  }

  // Operators ---------------------------------------------------------

  /**
    Checks the current time series for equality with another time
    series. This function is dumb and will really check for equal
    values up to machine precision.
  */

  bool operator==( const TimeSeries& other ) const noexcept;
  bool operator!=( const TimeSeries& other ) const noexcept;

  /**
    Checks whether two time series of the same length are *close*
    to each, i.e. their values are within a pre-defined tolerance
    threshold. This is used to remove duplicates.

    The implementation follows `numpy.isclose()`.
  */

  bool isClose( const TimeSeries& other, double rtol = 1e-6, double atol = 1e-8 ) const noexcept;

  // Iterators ---------------------------------------------------------
  //
  // Opting for in-line implementations here because it is not worth the
  // extra trouble.

  const_iterator begin() const noexcept { return _values.begin(); }
  iterator       begin()       noexcept { return _values.begin(); }

  const_iterator end()   const noexcept { return _values.end(); }
  iterator       end()         noexcept { return _values.end(); }

  // Access ------------------------------------------------------------

  const ValueType& operator[]( std::size_t index ) const noexcept { return _values[ index ]; }
  ValueType&       operator[]( std::size_t index )       noexcept { return _values[ index ]; }

  const ValueType& at( std::size_t index ) const { return _values.at( index ); }
  ValueType&       at( std::size_t index )       { return _values.at( index ); }

  void pop_front() { _values.erase( _values.begin() ); }
  void pop_back()  { _values.pop_back();               }

  // Attributes --------------------------------------------------------

  /** Returns current size or length of time series */
  std::size_t size()   const noexcept { return _values.size(); }
  std::size_t length() const noexcept { return _values.size(); }

  /** Checks whether the current time series is empty */
  bool empty() const noexcept { return _values.empty(); }

  // Distance calculations ---------------------------------------------

  ValueType distance( const TimeSeries& other ) const noexcept;
  ValueType distanceCentred( const TimeSeries& other ) const noexcept;

private:
  std::vector<ValueType> _values;
};

/**
  Reads a single time series from a given input stream. The appropriate
  error flags will be set if no time series can be read. Separators for
  the time series will be tested automatically:

    - `,`
    - `;`
    - `:`
    - ` `

  The list of separators is subject to change. Notice that the function
  expects time series to be specified line by line.
*/

std::istream& operator>>( std::istream& in, TimeSeries& T );

/** Basic output operator for a time series. */
std::ostream& operator<<( std::ostream& out, const TimeSeries& T );

#endif
