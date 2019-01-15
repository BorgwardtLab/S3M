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

# Include information about the CMake environment. This is required for
# tests.
CONFIGURE_FILE( include/Environment.hh.in
  ${CMAKE_SOURCE_DIR}/include/Environment.hh
)

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
# Setup versioning
########################################################################

GET_GIT_COMMIT_ID()

CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/include/Version.hh.in
  ${CMAKE_SOURCE_DIR}/include/Version.hh
)

########################################################################
# Dependencies
########################################################################

FIND_PACKAGE( Threads )
FIND_PACKAGE( Boost REQUIRED
  COMPONENTS
    date_time
    log
    program_options
    system
    timer
    thread
)

########################################################################
# Include directories
########################################################################

INCLUDE_DIRECTORIES(
  # Specifying these directories to represent `SYSTEM` headers as this
  # ensures that we do *not* get any warnings for them.
  SYSTEM
    ${Boost_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  # For these directories, we *expect* and *want* warnings to be shown
  # during the compilation process.
  ${CMAKE_SOURCE_DIR}/include
)

########################################################################
# Main executable(s)
########################################################################

ADD_EXECUTABLE( s3m
  s3m.cc
  #
  source/ContingencyTable.cc
  source/ContingencyTables.cc
  source/LookupTable.cc
  source/PiecewiseLinearFunction.cc
  source/ProgressDisplay.cc
  source/SignificantShapelets.cc
  source/SlidingWindow.cc
  source/TimeSeries.cc
  source/Utilities.cc
  #
  source/distances/DistanceFunctor.cc
  source/distances/Lp.cc
  source/distances/Minkowski.cc
)

# This ensures that the runtime path will be set correctly for the
# installed binary. Else, we run into problems if multiple library
# versions are installed.
SET_TARGET_PROPERTIES( s3m
  PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
)

TARGET_COMPILE_OPTIONS( s3m
  PRIVATE
    "-DBOOST_LOG_DYN_LINK"
    "-O3"
)

TARGET_LINK_LIBRARIES( s3m ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

INSTALL( TARGETS s3m
  RUNTIME
  DESTINATION bin
)

########################################################################
# Testing
########################################################################

ENABLE_TESTING()
ADD_SUBDIRECTORY( tests )

########################################################################
# Packaging
########################################################################

SET( BUILD_LINUX_PACKAGES
  "OFF"
  CACHE BOOL "Build binary packages for Linux distributions"
)

IF( BUILD_LINUX_PACKAGES AND UNIX AND NOT APPLE )
  MESSAGE( STATUS "Attempting to build binary packages for Linux distributions" )

  # Building a Debian package requires `dpkg-deb`
  FIND_PROGRAM( DPKG_COMMAND dpkg-deb )
  IF( DPKG_COMMAND )
    MESSAGE( STATUS "Found dpkg-deb: ${DPKG_COMMAND}" )
    MESSAGE( STATUS "Attempting to build Debian (.deb) package" )

    FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/packages/s3m-master )
    FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/packages/s3m-master/DEBIAN )
    FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/packages/s3m-master/usr/local/bin )

    CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/packages/DEBIAN/control.in
      ${CMAKE_BINARY_DIR}/packages/s3m-master/DEBIAN/control
    )

    # Copy the executable into the directory for the Debian package file,
    # which only works *after* the build process has finished.
    ADD_CUSTOM_COMMAND(
      TARGET s3m POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
              ${CMAKE_BINARY_DIR}/s3m
              ${CMAKE_BINARY_DIR}/packages/s3m-master/usr/local/bin/s3m
    )

    # Create the package *after* all other steps have been performed. This
    # step *must* be run `POST_BUILD` because the executable will *not* be
    # available prior to that.
    ADD_CUSTOM_COMMAND(
      TARGET s3m POST_BUILD
      COMMAND ${DPKG_COMMAND} --build s3m-master
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/packages
    )
  ENDIF()
ENDIF()
