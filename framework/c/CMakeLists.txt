#*******************************************************************************
# Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
#
# Contributors:
#     Sierra Wireless - initial API and implementation
#*******************************************************************************

set(LEGATO_C_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# When building for an embedded target, don't include the build output library directory
# in executables' DT_RUNPATH (and DT_RPATH).
if(LEGATO_EMBEDDED)
    set(CMAKE_SKIP_BUILD_RPATH 1)
endif()

# Ensure that DT_RUNPATH is used to override DT_RPATH (which is deprecated) in ELF executables
# when available for the target.  This allows the library search path to be overridden using the
# LD_LIBRARY_PATH environment variable in emergencies.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")

# Includes
include_directories(inc)
include_directories(src/logDaemon)
include_directories(src/security)
include_directories(src/supervisor)
include_directories(src/serviceDirectory)

# Definitions
set(LEGATO_FRAMEWORK_NICE_LEVEL "$ENV{LEGATO_FRAMEWORK_NICE_LEVEL}")
set(LE_RUNTIME_DIR "$ENV{LE_RUNTIME_DIR}")

set(LE_SVCDIR_SERVER_SOCKET_NAME "$ENV{LE_SVCDIR_SERVER_SOCKET_NAME}")
add_definitions(-DLE_SVCDIR_SERVER_SOCKET_NAME="${LE_SVCDIR_SERVER_SOCKET_NAME}")

set(LE_SVCDIR_CLIENT_SOCKET_NAME "$ENV{LE_SVCDIR_CLIENT_SOCKET_NAME}")
add_definitions(-DLE_SVCDIR_CLIENT_SOCKET_NAME="${LE_SVCDIR_CLIENT_SOCKET_NAME}")


# Subdirectories
add_subdirectory(liblegato)
add_subdirectory(src/logDaemon)
add_subdirectory(src/supervisor)
add_subdirectory(src/serviceDirectory)
add_subdirectory(src/configTree)
add_subdirectory(src/watchdog)

# Tests
enable_testing()
set(EXECUTABLE_OUTPUT_PATH ${LEGATO_BINARY_DIR}/bin/tests)

add_subdirectory(test/clock)
add_subdirectory(test/lists)
add_subdirectory(test/memPool)
add_subdirectory(test/utf8)

