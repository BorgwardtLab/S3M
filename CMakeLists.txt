CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
PROJECT( S3M CXX )

IF( CMAKE_MAJOR_VERSION GREATER 3 OR ( CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION GREATER 0 ) )
  SET( CMAKE_CXX_STANDARD 11 )
  SET( CMAKE_CXX_STANDARD_REQUIRED ON )
ELSE()
  # This is not the nicest way of activating C++11, but it is guaranteed to
  # work with older versions
  ADD_DEFINITIONS( "-std=c++11" )
ENDIF()

########################################################################
# General
########################################################################

MESSAGE( STATUS "CMAKE_ROOT:           ${CMAKE_ROOT}" )
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}" )

########################################################################
# Macros and additional flags
########################################################################

INCLUDE( CheckCXXCompilerFlag )

FILE( GLOB filenames "cmake/Macros/*.cmake" )
FOREACH( filename ${filenames} )
  INCLUDE( ${filename} )
ENDFOREACH()

ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wall" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wconversion" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wextra" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wnon-virtual-dtor" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wold-style-cast" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Woverloaded-virtual" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wself-init" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-Wunsafe-loop-optimization" )
ENABLE_IF_SUPPORTED( CMAKE_CXX_FLAGS "-pedantic" )

########################################################################
# Dependencies
########################################################################

FIND_PACKAGE( Boost REQUIRED COMPONENTS program_options system timer )

########################################################################
# Include directories
########################################################################

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

ADD_SUBDIRECTORY( source )

########################################################################
# Main executable(s)
########################################################################

ADD_EXECUTABLE( s3m
  s3m.cc
  #
  source/ContingencyTable.cc
  source/ContingencyTables.cc
  source/ProgressDisplay.cc
  source/SignificantShapelets.cc
  source/SlidingWindow.cc
  source/TimeSeries.cc
  source/Utilities.cc
)

TARGET_COMPILE_OPTIONS( s3m
  PRIVATE
    "-O3"
)

TARGET_LINK_LIBRARIES( s3m ${Boost_LIBRARIES} )
