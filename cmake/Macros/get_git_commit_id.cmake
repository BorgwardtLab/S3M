# Macro : get_git_commit_id.cmake
# Author: Bastian Rieck <bastian.rieck@bsse.ethz.ch>
#
# Provides the macro GET_GIT_COMMIT_ID, which looks up the last SHA-1
# ID of the last commmit in the repository. This is used to provide a
# consistent versioning for the results.
#
# If the `git` executable cannot be found, the macro returns an empty
# commmit ID.
#
# The commit ID can be accessed in the `GIT_COMMIT_ID` variable which
# is part of the parent scope.
#
# Usage:
#   GET_GIT_COMMIT_ID( variable )

MACRO( GET_GIT_COMMIT_ID )
  FIND_PROGRAM( GIT_PATH
    git
  )

  IF( GIT_PATH )
    EXECUTE_PROCESS(
      COMMAND ${GIT_PATH} log -1 --format=%h
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE COMMIT_ID
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET( GIT_COMMIT_ID ${COMMIT_ID} )
  ELSE()
    SET( GIT_COMMIT_ID "" )
  ENDIF()
ENDMACRO()
