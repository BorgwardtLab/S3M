#include "ProgressDisplay.hh"

#include <iomanip>

ProgressDisplay::ProgressDisplay( std::size_t N,
                                  std::ostream& out )
  : _N( N )
  , _n( 0 )
  , _t( 0 )
  , _T( 0 )
  , _out( out )
{
}

std::size_t ProgressDisplay::operator+=( std::size_t increment )
{
  _n += increment;
  if( _n >= _T )
    this->update();

  return _n;
}

std::size_t ProgressDisplay::operator++()
{
  return this->operator+=( 1 );
}

void ProgressDisplay::clear()
{
  _out << "\033[100D" // move cursor back 100 columns
       << "\033[2K"   // delete until end of line
       << "\x1B[A"    // move one line up
       << "\033[2K"   // delete until end of line
       << "\x1B[A"    // move one line up
       << "\033[2K"   // delete until end of line
       << "\x1B[A"    // move one line up
       << "\033[2K"   // delete until end of line
       << "\x1B[A"    // move one line up
       << "\033[2K";  // delete until end of line


  for( auto&& pair : _fields )
  {
    // Silence compiler warnings about unused variables; we only need
    // the variable to iterate over all fields.
    (void) pair;

    _out << "\x1B[A"    // move one line up
         << "\033[2K";  // delete until end of line
  }
}

void ProgressDisplay::draw()
{
  _t = _T = 0;

  _out << std::string( 51, '-' ) << "\n";

  for( auto it = _fields.begin(); it != _fields.end(); ++it )
  {
    _out << std::left
         << std::setfill( ' ' )
         << std::setw( 18 )
         << it->first << " = ";

    auto value = it->second;
    if( value.type() == typeid(long double) )
      _out << std::setprecision( 2 ) << boost::any_cast<long double>( value );
    else if( value.type() == typeid(double) )
      _out << std::setprecision( 2 ) << boost::any_cast<double>( value );
    else if( value.type() == typeid(std::size_t) )
      _out << boost::any_cast<std::size_t>( value );
    else if( value.type() == typeid(unsigned) )
      _out << boost::any_cast<std::size_t>( value );

    _out << "\n";
  }

  _out << std::string( 51, '-' ) << "\n"
       << "0%   10   20   30   40   50   60   70   80   90   100%\n"
       << "|----|----|----|----|----|----|----|----|----|----|"
       << std::endl;

  this->update();
}

void ProgressDisplay::update()
{
  // Do not draw any tics if there is an insufficient amount of data
  // available. Else, we divide by zero in the line below.
  if( _n == 0 || _N == 0 )
    return;

  std::size_t requiredTics
    = static_cast<std::size_t>( (static_cast<double>( _n ) / static_cast<double>( _N ) ) * 50.0 );

  do
  {
    _out << '*' << std::flush;
  }
  while ( ++_t < requiredTics );

  _T = static_cast<std::size_t>( ( static_cast<double>( _t ) / 50.0 ) * static_cast<double>( _N ) );
  if( _n == _N )
  {
    if( _t < 51 )
     _out << '*';

    _out << std::endl;
  }
}
