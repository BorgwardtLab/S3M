#ifndef UTILITIES_HH__
#define UTILITIES_HH__

/*
  Copyright (c) 2018 Bastian Rieck

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "TimeSeries.hh"

#include <algorithm>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <regex>
#include <utility>
#include <vector>

#include <cctype>

/**
  Performs an argument sort of a vector, i.e. sorting by indices instead
  of values stored in the vector itself. The name of the function has of
  course been stolen from `numpy.argsort()`.
*/

template <class T> std::vector<std::size_t> argsort( const std::vector<T>& data )
{
  std::vector<std::size_t> indices( data.size() );
  std::iota( indices.begin(), indices.end(), 0 );

  std::sort( indices.begin(), indices.end(),
             [&data] ( std::size_t i, std::size_t j )
             {
               return data[i] < data[j];
             }
  );

  return indices;
}

/**
  Permits splitting a generic sequence of characters using a regular
  expression. The default mode is to split by whitespace characters,
  but this can be overridden by specifying a different regex.
*/

template <class T> std::vector<T> split( const T& sequence,
                                         const T& regex = "[[:space:]]+" )
{
  std::regex re( regex );
  std::sregex_token_iterator begin( sequence.begin(), sequence.end(), re, -1 );
  std::sregex_token_iterator end;

  return { begin, end };
}

/**
  Attempts to convert a sequence type `S` to a non-sequence type `T` by
  using `std::stringstream`. This makes converting strings to different
  types such as numbers easier.

  @tparam S Sequence type (e.g. `std::string`)
  @tparam T Non-sequence type (e.g. `ìnt`)

  @param sequence Sequence to convert
  @param success  Flag indicating the success of the conversion

  @returns Result of the conversion. Errors do *not* result in an error
  being thrown. Use the \p success parameter to check for errors.
*/

template <class T, class S> T convert( const S& sequence, bool& success )
{
  T result = T();
  success  = false;

  std::istringstream converter( sequence );
  converter >> result;

  // Try some special handling for some special tokens. Other errors are
  // silently ignored. I am not sure whether this is the right behaviour
  // but I see no pressing reason to change it now.
  if( converter.fail() )
  {
    std::string string = sequence;

    std::transform( string.begin(), string.end(),
                    string.begin(), ::tolower );

    if( string == "+inf" || string == "inf" || string == "+infinity" || string == "infinity" )
      result = std::numeric_limits<T>::infinity();
    else if ( string == "-inf" || string == "-infinity" )
      result = -std::numeric_limits<T>::infinity();
    else if( string == "nan" )
      result = std::numeric_limits<T>::quiet_NaN();

    success = result != T();
  }
  else
    success = true;

  return result;
}

/** @overload convert() */
template <class T, class S> T convert( const S& sequence )
{
  bool success = false;
  return convert<T>( sequence, success );
}

/**
  Loads a set of time series from a data file. The data file is supposed
  to contain a single time series per line. The label of the time series
  must be encoded somewhere in the time series. Typically, this function
  uses the *first* value for this purpose.

  It is assumed that only two labels exist.

  @param filename       Input filename
  @param l              Label index, i.e. the position of the time series that
                        contains the label
  @param excludeColumns Column indices to exclude from the calculations
*/

std::pair< std::vector<TimeSeries>, std::vector<bool> > readData( const std::string& filename, unsigned l = 0, const std::vector<unsigned>& excludeColumns = std::vector<unsigned>() );

/**
  Performs $z$-score normalization, i.e. standardization, of a set of
  time series. They will be adjusted such that their mean is zero and
  they have unit variance.

  The function will return the sample mean and the sample variance in
  order to make it possible to *reverse* the transformation.
*/

std::pair<double, double> standardizeData( std::vector<TimeSeries>& timeSeries );

#endif
