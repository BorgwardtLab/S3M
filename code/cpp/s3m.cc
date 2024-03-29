#include "SignificantShapelets.hh"
#include "TimeSeries.hh"
#include "Utilities.hh"
#include "Version.hh"

#include "distances/DistanceFunctor.hh"
#include "distances/Lp.hh"
#include "distances/Minkowski.hh"

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
  // Silence compiler warnings if Boost.Log is not present in the
  // platform.
  (void) r;
  (void) o;
}

void setupLogging()
{
  namespace logging = boost::log;
  auto core         = logging::core::get();

  logging::add_common_attributes();

  boost::shared_ptr<logging::sinks::text_ostream_backend> backend
    = boost::make_shared<logging::sinks::text_ostream_backend>();

  backend->add_stream(
    boost::shared_ptr<std::ostream>( &std::clog, []( std::ostream* ) {} )
  );

  backend->auto_flush( true );

  using Sink = logging::sinks::synchronous_sink<logging::sinks::text_ostream_backend>;
  boost::shared_ptr<Sink> sink( new Sink( backend ) );

  sink->set_formatter( &formatLogRecord );
  core->add_sink( sink );
}

std::shared_ptr<DistanceFunctor> selectDistance( const std::string& name )
{
  auto position = name.find( ':' );
  auto metric   = name.substr( 0, position );
  auto power    = position != std::string::npos ? name.substr( position + 1 ) : std::string();
  auto p        = power.empty() ? 2.0 : std::stod( power );

  if( metric == "minkowski" )
    return std::make_shared<MinkowskiDistance>( p );

  else if( metric == "lp" )
    return std::make_shared<LpDistance>( p );

  // Fall back to the default distance here instead of selecting one
  // that does not fit.
  return std::make_shared<MinkowskiDistance>( 2.0 );
}

int main( int argc, char** argv )
{
  setupLogging();

  using namespace boost::program_options;

  bool allShapelets         = false;
  bool standardize          = false;
  bool disablePruning       = false;
  bool mergeTables          = false;
  bool quiet                = false;
  bool removeDuplicates     = false;
  bool withPseudocounts     = false;

  unsigned m = 0; // minimum pattern length
  unsigned M = 0; // maximum pattern length
  unsigned s = 0; // stride
  unsigned l = 0; // label index in time series
  unsigned k = 0; // number of significant shapelets to keep

  std::string excludeColumns;
  std::string distance;
  std::string input;
  std::string output = "-";

  options_description description( "Available options" );
  description.add_options()
    ("help,h"                 , "Show help")
    ("standardize"            , "Standardize data" )
    ("all,a"                  , "Report all shapelets, not just the most significant ones")
    ("merge-tables,t"         , "Merge equal contingency tables")
    ("disable-pruning,p"      , "Disable pruning criterion" )
    ("remove-duplicates,r"    , "Remove duplicates" )
    ("with-pseudocounts,c"    , "Use pseudocounts in contingency tables" )
    ("quiet,q"                , "Disables progress bar" )
    ("min-length,m"           , value<unsigned>( &m )->default_value( 10 ), "Minimum candidate pattern length" )
    ("max-length,M"           , value<unsigned>( &M )->default_value(  0 ), "Maximum candidate pattern length" )
    ("stride,s"               , value<unsigned>( &s )->default_value(  1 ), "Stride" )
    ("label-index,l"          , value<unsigned>( &l )->default_value(  0 ), "Index of label in time series" )
    ("keep,k"                 , value<unsigned>( &k )->default_value(  0 ), "Maximum number of shapelets to keep (0 = unlimited" )
    ("distance,d"             , value<std::string>( &distance )           , "Use non-standard distance function")
    ("exclude-columns,e"      , value<std::string>( &excludeColumns )     , "Columns to exclude for shapelet processing" )
    ("input,i"                , value<std::string>( &input )              , "Training file" )
    ("output,o"               , value<std::string>( &output )             , "Output file (specify '-' for stdout)" );

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

  BOOST_LOG_TRIVIAL(info) << "S3M (" << GIT_COMMIT_ID << ")";

  if( variables.count("all") )
  {
    allShapelets = true;

    BOOST_LOG_TRIVIAL(warning) << "You are requesting *all* shapelets to be reported, even insignificant ones";
  }

  if( variables.count("disable-pruning") )
    disablePruning = true;

  if( variables.count("merge-tables") )
    mergeTables = true;

  if( variables.count("quiet") )
    quiet = true;

  if( variables.count("remove-duplicates") )
    removeDuplicates = true;

  if( variables.count("standardize") )
    standardize = true;

  if( variables.count( "with-pseudocounts" ) )
    withPseudocounts = true;

  // Show usage information if no input file has been specified. This is
  // nicer than just ending the program with an exception.
  if( input.empty() )
  {
    std::cerr << "No input file was specified. S3M needs at least an input file\n"
              << "to perform shapelet extraction.\n\n"
              << description << "\n";

    return 0;
  }

  // 1. Read training data ---------------------------------------------

  BOOST_LOG_TRIVIAL(info) << "Loading input from " << input;

  std::vector<unsigned> excludedColumns;
  if( !excludeColumns.empty() )
  {
    BOOST_LOG_TRIVIAL(info) <<  "Excluding the following columns in addition to the label column: "
                            << excludeColumns;

    // Excluded columns can be separated by commas, colons, and
    // semicolons, or a mixture of them.
    auto tokens = split( excludeColumns, std::string( "[,:;]+" ) );
    for( auto&& token : tokens )
    {
      auto value = convert<unsigned>( token );
      excludedColumns.emplace_back( value );
    }
  }

  auto data         = readData( input, l, excludedColumns );
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

  // Only set the distance if the user requested a specific one. Else,
  // the default implementation is much faster.
  if( !distance.empty() )
    significantShapelets.setDistance( selectDistance( distance) );

  significantShapelets.disablePruning( disablePruning );             // enable/disable pruning
  significantShapelets.mergeTables( mergeTables );                   // enable/disable merging of contingency tables
  significantShapelets.quiet( quiet );                               // enable/disable progress bar display
  significantShapelets.removeDuplicates( removeDuplicates );         // enable/disable duplicate removal upon extraction
  significantShapelets.reportAllShapelets( allShapelets );           // enable/disable pruning based on significance threshold
  significantShapelets.withPseudocounts( withPseudocounts );         // enable use of pseudocounts for contingency tables

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

  if( withPseudocounts )
    BOOST_LOG_TRIVIAL(info) << "Using pseudocounts for contingency table calculation";

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
         << "    \"standardize\": " << std::boolalpha << standardize << std::noboolalpha << ",\n";

    // Only add this section if we deviated from the default distance.
    // We do not want to clutter up the output.
    if( !distance.empty() )
    {
      // This is a *little* bit inefficient since we already set the
      // functor above, but I do not want to write yet another 'get'
      // function for the significant shapelets class.
      auto functor = selectDistance( distance);

      *out << "    \"distance\": " << "\"" << functor->name() << "\",\n";
    }

    *out << "    \"p_tarone\": "    << p_tarone << ",\n"
         << "    \"version\": "     << "\"" << GIT_COMMIT_ID << "\"\n"
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
