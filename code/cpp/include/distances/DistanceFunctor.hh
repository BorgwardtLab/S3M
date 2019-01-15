#ifndef DISTANCE_FUNCTOR_HH__
#define DISTANCE_FUNCTOR_HH__

#include <string>

// Using a forward reference because it is sufficient here, thus
// minimizing the complexity of all dependencies to some extent.
class TimeSeries;

class DistanceFunctor
{
public:

  // Forwards the return value type of this functor to make it easier to
  // remain type-agnostic.
  using ValueType = double;

  // A default constructor is sufficient, as this class is not supposed
  // to carry any significant amounts of data.
  DistanceFunctor() = default;

  // Ditto for the destructor.
  virtual ~DistanceFunctor() = default;

  /**
    Main entry point for the functor. This calculates the distance
    between two time series.
  */

  virtual ValueType operator()( const TimeSeries& S, const TimeSeries& T ) const = 0; 

  /**
    Returns the name of the distance functor. Ideally, this should
    include parameters, but this function cannot enforce it.
  */

  virtual std::string name() const = 0;
};

#endif
