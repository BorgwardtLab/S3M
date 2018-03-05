#ifndef SIGNIFICANT_SHAPELETS_HH__
#define SIGNIFICANT_SHAPELETS_HH__

#include "ContingencyTable.hh"
#include "TimeSeries.hh"

#include <initializer_list>
#include <iosfwd>
#include <vector>

/**
  @class SignificantShapelets
  @brief Main extraction class for significant shapelets

  This is the main class for performing the extraction of significant
  shapelets from a given set of time series. Its entry point is given
  by `operator()`, in which the extraction process is performed. Note
  that the implementation of `operator()` follows the pseudocode from
  the paper.
*/

class SignificantShapelets
{
public:

  using ValueType = typename TimeSeries::ValueType;

  /** Data structure for describing the result of the extraction process */
  struct SignificantShapelet
  {
    TimeSeries shapelet;    // the extracted shapelet
    long double p;          // $p$-value
    ContingencyTable table; // best contingency table
  };

  // Constructors ------------------------------------------------------

  /**
    Creates a new instance of a significant shapelet extractor with
    a fixed window size and stride for candidate generation.
  */

  SignificantShapelets( unsigned size, unsigned windowStride );

  /**
    Creates a new instances of a significant shapelet extractor with
    a predefined minimum/maximum window size and a window stride for
    candidate generation.
  */

  SignificantShapelets( unsigned minSize, unsigned maxSize, unsigned windowStride );

  // Parameters --------------------------------------------------------

  template <class InputIterator> void setDistances( InputIterator begin, InputIterator end )
  {
    _distances.assign( begin, end );
  }

  void setDistances( std::initializer_list<ValueType>&& il )
  {
    _distances.assign( il.begin(), il.end() );
  }

  void disablePruning( bool value = true ) noexcept
  {
    _disablePruning = value;
  }

  void removeDuplicates( bool value = true ) noexcept
  {
    _removeDuplicates = value;
  }

  // Extraction --------------------------------------------------------

  /**
    Performs the extraction process with the given settings for a set of
    time series with a set of associated labels.
  */

  std::vector<SignificantShapelet> operator()( const std::vector<TimeSeries>& timeSeries,
                                               const std::vector<bool>& labels,
                                               long double& tarone,
                                               std::vector<long double>& thresholds );

private:

  /**
    Calculates the minimum attainable $p$-values for a given problem
    size. This is required in order to perform the adjustment of the
    $p$-value following Tarone's method.

    The returned values will be sorted in ascending order. Hence, it
    is required to access elements from the back of the vector.
  */

  static std::vector<long double> min_attainable_p_values( unsigned n, unsigned n1 );

  unsigned _minWindowSize;
  unsigned _maxWindowSize;
  unsigned _windowStride;

  bool _removeDuplicates = false;
  bool _disablePruning   = false;

  // Target FWER before any adjustments of the threshold are being made
  // using Tarone's method.
  double _alpha = 0.05;

  // If set, uses these distances as the distance thresholds for which
  // to perform all calculations.
  std::vector<ValueType> _distances;
};

/**
  Adds a significant shapelet to an output stream. JSON is used to
  format the output.
*/

std::ostream& operator<<( std::ostream& o, const SignificantShapelets::SignificantShapelet& ss );

#endif
