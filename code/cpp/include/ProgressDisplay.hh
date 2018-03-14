#ifndef PROGRESS_DISPLAY_HH__
#define PROGRESS_DISPLAY_HH__

#include <iostream>
#include <map>
#include <ostream>
#include <string>

#include <cstddef>

#include <boost/any.hpp>

/**
  @class ProgressDisplay
  @brief A progress display for the command line

  This is a simple "visualization" of the progress of an operation, such
  as the extraction of significant shapelets, for command-line tools. It
  is based on an example of the Boost library.
*/

class ProgressDisplay
{
public:
  ProgressDisplay( std::size_t N,
                   std::ostream& out = std::cerr );

  void addField( const std::string& name )
  {
    _fields[name] = boost::any();
  }

  template <class T> void setField( const std::string& name, const T& value )
  {
    _fields.at(name) = boost::any( value );

    this->clear();
    this->draw();
  }

  std::size_t operator+=( std::size_t increment );
  std::size_t operator++();

  void clear();
  void draw();
  void update();

private:
  std::size_t _N;     // maximum allowed/expected count
  std::size_t _n;     // current count
  std::size_t _t;     // current tic position (for drawing)
  std::size_t _T;     // next tic position (for drawing)
  std::ostream& _out; // output stream

  /** Maps display fields to their current values */
  std::map<std::string, boost::any> _fields;
};

#endif
