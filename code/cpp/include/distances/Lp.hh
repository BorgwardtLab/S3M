#ifndef DISTANCES_LP_HH__
#define DISTANCES_LP_HH__

#include "DistanceFunctor.hh"

class LpDistance : public DistanceFunctor
{
public:
  using ValueType = DistanceFunctor::ValueType;

  LpDistance( ValueType p );

  virtual ValueType operator()( const TimeSeries& S, const TimeSeries& T ) const;

  virtual std::string name() const noexcept
  {
    return "Lp:" + std::to_string( _p );
  }

private:
  ValueType _p;
};

#endif
