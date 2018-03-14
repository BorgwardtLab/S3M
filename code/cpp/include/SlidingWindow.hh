#ifndef SLIDING_WINDOW_HH__
#define SLIDING_WINDOW_HH__

#include <vector>

// A forward declaration is sufficient here because we are only
// *declaring* a function below.
class TimeSeries;

/*
  Class for extracting candidate shapelets by going over them using
  a sliding window approach. The class offers various configuration
  options that influence the extraction process.
*/

class SlidingWindow
{
public:

  /**
    Creates a new sliding window extractor with some pre-defined
    settings. The window size remains *fixed* during extraction.
  */

  SlidingWindow( unsigned size, unsigned stride );

  /**
    Creates a new sliding window extractor with some pre-defined
    settings. Here, the window size *varies* during extraction.
  */

  SlidingWindow( unsigned minSize, unsigned maxSize, unsigned stride );

  // Attributes --------------------------------------------------------

  unsigned minSize() const noexcept { return _minSize; }
  unsigned maxSize() const noexcept { return _maxSize; }
  unsigned stride()  const noexcept { return _stride;  }

  void setRemoveDuplicates( bool value = true );
  bool removeDuplicates() const noexcept;

  // Extraction --------------------------------------------------------

  /**
    Main extraction routine. Uses the current settings of the sliding
    window extractor to go over a time series. Candidate patterns are
    stored and will be returned in the order in which they appear.
  */

  std::vector<TimeSeries> operator()( const TimeSeries& T ) const noexcept;

private:
  unsigned _minSize      = 0;     // minimum window size
  unsigned _maxSize      = 0;     // maximum window size
  unsigned _stride       = 1;     // stride

  bool _removeDuplicates = false; // flag indicating whether duplicates should be removed
};

#endif
