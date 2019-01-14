#ifndef DISTANCES_MINKOWSKI_HH__
#define DISTANCES_MINKOWSKI_HH__

#include "DistanceFunctor.hh"

class MinkowskiDistance : public DistanceFunctor
{
public:
  using ValueType = DistanceFunctor::ValueType;

  MinkowskiDistance( ValueType p );

  virtual ValueType operator()( const TimeSeries& S, const TimeSeries& T );

private:
  ValueType _p = ValueType();
};

#endif
