cmake_minimum_required(VERSION 2.8)
project(duda C)

# Update CFLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

include(GNUInstallDirs)

# Find pthreads
find_package(Threads)

# Output paths
set(DUDA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/library")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/library")

# Fluent Bit Version
set(DUDA_VERSION_MAJOR  0)
set(DUDA_VERSION_MINOR  2)
set(DUDA_VERSION_PATCH  0)
set(DUDA_VERSION_STR "${DUDA_VERSION_MAJOR}.${DUDA_VERSION_MINOR}.${DUDA_VERSION_PATCH}")

# Build Options
option(DUDA_ALL               "Enable all features"          No)
option(DUDA_DEBUG             "Build with debug symbols"     No)
option(DUDA_TLS               "Build with SSL/TLS support"   No)
option(DUDA_TRACE             "Enable trace mode"            No)
option(DUDA_MTRACE            "Enable mtrace support"        No)

# Enable all features
if(DUDA_ALL)
endif()

# Macro to set definitions
macro(DUDA_DEFINITION var)
  add_definitions(-D${var})
  set(DUDA_BUILD_FLAGS "${DUDA_BUILD_FLAGS}#ifndef ${var}\n#define ${var}\n#endif\n")
  set(DUDA_INFO_FLAGS "${DUDA_INFO_FLAGS} ${var}")
endmacro()

# Enable Debug symbols if specified
if(DUDA_DEBUG)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# General Headers
include_directories(
  include/
  include/duda/
  deps/monkey/include/
  )

macro(DUDA_SET_OPTION option value)
  set(${option} ${value} CACHE "" INTERNAL FORCE)
endmacro()

# MK Core
DUDA_SET_OPTION(MK_DEBUG              ON)
DUDA_SET_OPTION(MK_SYSTEM_MALLOC      ON)
DUDA_SET_OPTION(MK_WITHOUT_BIN        ON)
DUDA_SET_OPTION(MK_WITHOUT_CONF       ON)
DUDA_SET_OPTION(MK_PLUGIN_AUTH       OFF)
DUDA_SET_OPTION(MK_PLUGIN_CGI        OFF)
DUDA_SET_OPTION(MK_PLUGIN_CHEETAH    OFF)
DUDA_SET_OPTION(MK_PLUGIN_DIRLISTING OFF)
DUDA_SET_OPTION(MK_PLUGIN_FASTCGI    OFF)
DUDA_SET_OPTION(MK_PLUGIN_LOGGER     OFF)
DUDA_SET_OPTION(MK_PLUGIN_MANDRIL    OFF)
DUDA_SET_OPTION(MK_PLUGIN_TLS        OFF)
add_subdirectory(deps/monkey)

if(DUDA_TRACE)
  DUDA_DEFINITION(DUDA_HAVE_TRACE)
endif()

# Valgrind support
check_c_source_compiles("
  #include <valgrind/valgrind.h>
  int main() {
     return 0;
  }" DUDA_HAVE_VALGRIND)

if(DUDA_VALGRIND OR DUDA_HAVE_VALGRIND)
  DUDA_DEFINITION(DUDA_HAVE_VALGRIND)
endif()

# mtrace support
if(DUDA_MTRACE)
  check_c_source_compiles("
    #include <mcheck.h>
    int main() {
       return 0;
    }" DUDA_HAVE_MTRACE)

  if(DUDA_HAVE_MTRACE AND DUDA_DEBUG)
    DUDA_DEFINITION(DUDA_HAVE_MTRACE)
  endif()
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/include/duda/duda_info.h.in"
  "${PROJECT_SOURCE_DIR}/include/duda/duda_info.h"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/include/duda/duda_version.h.in"
  "${PROJECT_SOURCE_DIR}/include/duda/duda_version.h"
  )

add_subdirectory(lib)
add_subdirectory(src)
