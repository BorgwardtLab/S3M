#include "SignificantShapelets.hh"
#include "TimeSeries.hh"
#include "Utilities.hh"

#include <boost/core/null_deleter.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/make_shared.hpp>

#include <boost/program_options.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/timer/timer.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include <cmath>

// Adds colours to the output of the log. This is based on an example
// from StackOverflow [1].
//
// [1]: https://stackoverflow.com/a/38316911/1396991
void formatLogRecord( boost::log::record_view const& r, boost::log::formatting_ostream& o )
{
  namespace logging = boost::log;
  auto severity     = r[ logging::trivial::severity ];

  if( severity )
  {
    switch( severity.get() )
    {
    case logging::trivial::info:
      o << "\033[32m";
      break;
    case logging::trivial::warning:
      o << "\033[33m";
      break;
    case logging::trivial::error:
    case logging::trivial::fatal:
      o << "\033[31m";
      break;
    default:
      break;
    }
  }

  auto timeStamp = logging::extract<boost::posix_time::ptime>("TimeStamp", r);

  o << boost::posix_time::to_simple_string( *timeStamp ) << ": "
    << r[ logging::expressions::smessage ];

  // Restore the default colour
  if( severity )
    o << "\033[0m";
}

void setupLogging()
{
  namespace logging = boost::log;
  auto core         = logging::core::get();

  logging::add_common_attributes();

  boost::shared_ptr<logging::sinks::text_ostream_backend> backend
    = boost::make_shared<logging::sinks::text_ostream_backend>();

  backend->add_stream(
    boost::shared_ptr<std::ostream>( &std::clog, boost::null_deleter() )
  );

  backend->auto_flush( true );

  using Sink = logging::sinks::synchronous_sink<logging::sinks::text_ostream_backend>;
  boost::shared_ptr<Sink> sink( new Sink( backend ) );

  sink->set_formatter( &formatLogRecord );
  core->add_sink( sink );
}

int main( int argc, char** argv )
{
  setupLogging();

  using namespace boost::program_options;

  bool standardize      = false;
  bool disablePruning   = false;
  bool removeDuplicates = false;
  bool keepNormalOnly   = false;

  unsigned m = 0; // minimum pattern length
  unsigned M = 0; // maximum pattern length
  unsigned s = 0; // stride
  unsigned l = 0; // label index in time series
  unsigned k = 0; // number of significant shapelets to keep

  std::string input;
  std::string output = "-";

  options_description description( "Available options" );
  description.add_options()
    ("help,h"             , "Show help")
    ("standardize"        , "Standardize data" )
    ("keep-normal-only,n" , "Keep only normal p-values" )
    ("disable-pruning,p"  , "Disable pruning criterion" )
    ("remove-duplicates,r", "Remove duplicates" )
    ("min-length,m"       , value<unsigned>( &m )->default_value( 10 ), "Minimum candidate pattern length" )
    ("max-length,M"       , value<unsigned>( &M )->default_value(  0 ), "Maximum candidate pattern length" )
    ("stride,s"           , value<unsigned>( &s )->default_value(  1 ), "Stride" )
    ("label-index,l"      , value<unsigned>( &l )->default_value(  0 ), "Index of label in time series" )
    ("keep,k"             , value<unsigned>( &k )->default_value(  0 ), "Maximum number of shapelets to keep (0 = unlimited" )
    ("input,i"            , value<std::string>( &input )              , "Training file" )
    ("output,o"           , value<std::string>( &output )             , "Output file (specify '-' for stdout)" );

  positional_options_description positionalOptions;
  positionalOptions.add( "input", 1 );

  variables_map variables;

  store( command_line_parser( argc, argv )
      .options( description )
      .positional( positionalOptions )
      .run(),
    variables );
  notify( variables );

  if( variables.count("help") )
  {
    std::cerr << description << "\n";
    return 0;
  }

  if( variables.count("disable-pruning") )
    disablePruning = true;

  if( variables.count("keep-normal-only" ) )
    keepNormalOnly = true;

  if( variables.count("remove-duplicates") )
    removeDuplicates = true;

  if( variables.count("standardize") )
    standardize = true;

  // 1. Read training data ---------------------------------------------

  BOOST_LOG_TRIVIAL(info) << "Loading input from " << input;

  auto data         = readData( input, l );
  auto&& timeSeries = data.first;
  auto&& labels     = data.second;

  BOOST_LOG_TRIVIAL(info) << "Read "
                          << timeSeries.size()
                          << " time series from training input file";

  double mu    = 0.0;
  double sigma = 1.0;

  if( standardize )
  {
    BOOST_LOG_TRIVIAL(info) << "Starting data standardization";

    // Store the parameters in order to undo the standardization
    // procedure afterwards.
    std::tie( mu, sigma )
      = standardizeData( timeSeries );

    BOOST_LOG_TRIVIAL(info) << "Finished data standardization";
  }

  // 2. Perform the extraction -----------------------------------------

  boost::timer::cpu_timer timer;

  // Use a fixed window size if only a single parameter has been
  // specified
  if( m > M )
    M = m;

  BOOST_LOG_TRIVIAL(info) << "Extracting shapelets with length "
                          << "[" << m << ":" << M << "]";

  std::vector<long double> thresholds;
  SignificantShapelets significantShapelets( m, M, s );
  significantShapelets.disablePruning( disablePruning );      // enable/disable pruning
  significantShapelets.removeDuplicates( removeDuplicates );  // enable/disable duplicate removal upon extraction
  significantShapelets.keepNormalOnly( keepNormalOnly );      // enable/disable keeping normal $p$-values

  long double p_tarone = 0.0;
  auto shapelets       = significantShapelets( timeSeries,
                                               labels,
                                               p_tarone,
                                               thresholds );

  timer.stop();

  if( standardize )
  {
    using SignificantShapelet = SignificantShapelets::SignificantShapelet;

    std::transform(
      shapelets.begin(), shapelets.end(), shapelets.begin(),
        [&mu, &sigma] ( SignificantShapelet ss )
        {
          using ValueType  = typename TimeSeries::ValueType;
          auto && shapelet = ss.shapelet;

          std::transform( shapelet.begin(), shapelet.end(), shapelet.begin(),
            [&mu, &sigma] ( ValueType x )
            {
              return (x * sigma) + mu;
            }
          );

          return ss;
        }
    );
  }

  // 3. Output ---------------------------------------------------------
  //
  // The fiddling with the pointer below is only required in order to
  // be able to handle both file outputs and outputs to `stdout`. C++
  // makes this a very strange case to handle.

  {
    std::ostream* out = &std::cout;
    std::ofstream fout;
    if( output != "-" and not output.empty() )
    {
      fout.open( output );
      out = &fout;
    }

    *out << std::setprecision( 32 );

    *out << "{\n"
         << "  \"parameters\": {\n"
         << "    \"min_length\": "  << m << ",\n"
         << "    \"max_length\": "  << M << ",\n"
         << "    \"stride\": "      << s << ",\n"
         << "    \"standardize\": " << std::boolalpha << standardize << std::noboolalpha << ",\n"
         << "    \"p_tarone\": "    << p_tarone << "\n"
         << "  },\n"
         << "  \"shapelets\": [\n";

    if( k != 0 && k < shapelets.size() )
      shapelets.resize( k );

    for( auto it = shapelets.begin(); it != shapelets.end(); ++it )
    {
      if( it != shapelets.begin() )
        *out << ",\n";

      *out << *it;
    }

    *out << "  ]\n"
         << "}\n";
  }

  BOOST_LOG_TRIVIAL(info) << "Finished shapelet extraction. Total time:" << timer.format() << "\n";
}
