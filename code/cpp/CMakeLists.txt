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
    log
    program_options
    system
    timer
)

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
  source/LookupTable.cc
  source/ProgressDisplay.cc
  source/SignificantShapelets.cc
  source/SlidingWindow.cc
  source/TimeSeries.cc
  source/Utilities.cc
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
