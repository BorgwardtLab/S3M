#ifndef DISTANCES_MINKOWSKI_HH__
#define DISTANCES_MINKOWSKI_HH__

#include "DistanceFunctor.hh"

class MinkowskiDistance : public DistanceFunctor
{
public:
  using ValueType = DistanceFunctor::ValueType;

  MinkowskiDistance( ValueType p );

  virtual ValueType operator()( const TimeSeries& S, const TimeSeries& T ) const;

  virtual std::string name() const noexcept
  {
    return "Minkowski:" + std::to_string( _p );
  }

private:
  ValueType _p;
};

#endif
