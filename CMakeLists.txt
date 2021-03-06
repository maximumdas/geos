##############################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2018-2019 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
##############################################################################

# Require CMake 3.8+ with support for meta-features that request compiler
# modes for specific C/C++ language standard levels.
cmake_minimum_required(VERSION 3.8)

# Default to release build so packagers don't release debug builds
set(DEFAULT_BUILD_TYPE Release)

# Require CMake 3.13+ with VS generator for complete support of VS versions
# and support by AppVeyor.
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# TODO: Follow CMake detection of git and version tagging
#       https://gitlab.kitware.com/cmake/cmake/blob/master/Source/CMakeVersionSource.cmake
if(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD)
  set(GEOS_BUILD_FROM_GIT ON)
endif()

# Make sure we know our build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
  message(STATUS "GEOS: Using default build type: ${CMAKE_BUILD_TYPE}")
else()
  message(STATUS "GEOS: Build type: ${CMAKE_BUILD_TYPE}")
endif()


#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
include(CMakeDependentOption)

## CMake global variables
option(BUILD_SHARED_LIBS "Build GEOS with shared libraries" ON)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ standard version to use (default is 11)")

## GEOS custom variables
cmake_dependent_option(GEOS_BUILD_DEVELOPER
  "Build with compilation flags useful for development" ON
  "GEOS_BUILD_FROM_GIT" OFF)
mark_as_advanced(GEOS_BUILD_DEVELOPER)

if (POLICY CMP0092)
  # dont set /W3 warning flags by default, we already
  # set /W4 anyway
  cmake_policy(SET CMP0092 NEW)
endif()

#-----------------------------------------------------------------------------
# Setup build directories
#-----------------------------------------------------------------------------
# Place executables and shared libraries in the same location for
# convenience of direct execution from common spot and for
# convenience in environments without RPATH support.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
message(STATUS "GEOS: Run-time output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "GEOS: Archives output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

#-----------------------------------------------------------------------------
# Version
#-----------------------------------------------------------------------------
file(READ Version.txt _version_txt)

string(REGEX MATCH "GEOS_VERSION_MAJOR=([0-9]+)" _ ${_version_txt})
set(_version_major ${CMAKE_MATCH_1})
string(REGEX MATCH "GEOS_VERSION_MINOR=([0-9]+)" _ ${_version_txt})
set(_version_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "GEOS_VERSION_PATCH=([0-9]+)" _ ${_version_txt})
set(_version_patch ${CMAKE_MATCH_1})
# OPTIONS: "", "dev", "rc1" etc.
string(REGEX MATCH "GEOS_PATCH_WORD=([a-zA-Z0-9]+)" _ ${_version_txt})
set(_version_patch_word ${CMAKE_MATCH_1})

# Version of JTS this release is bound to
string(REGEX MATCH "JTS_PORT=([0-9a-zA-Z\.]+)" _ ${_version_txt})
set(JTS_PORT ${CMAKE_MATCH_1})

# Version of public C API
string(REGEX MATCH "CAPI_INTERFACE_CURRENT=([0-9]+)" _ ${_version_txt})
set(_version_capi_current ${CMAKE_MATCH_1})
string(REGEX MATCH "CAPI_INTERFACE_REVISION=([0-9]+)" _ ${_version_txt})
set(_version_capi_revision ${CMAKE_MATCH_1})
string(REGEX MATCH "CAPI_INTERFACE_AGE=([0-9]+)" _ ${_version_txt})
set(_version_capi_age ${CMAKE_MATCH_1})

unset(_version_txt)

math(EXPR _version_capi_major "${_version_capi_current} - ${_version_capi_age}")
set(CAPI_VERSION_MAJOR ${_version_capi_major})
set(CAPI_VERSION_MINOR ${_version_capi_age})
set(CAPI_VERSION_PATCH ${_version_capi_revision})
set(CAPI_VERSION "${_version_capi_major}.${_version_capi_age}.${_version_capi_revision}")

unset(_version_capi_current)
unset(_version_capi_major)
unset(_version_capi_age)
unset(_version_capi_revision)

#-----------------------------------------------------------------------------
# Project
#-----------------------------------------------------------------------------
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.9)
  list(APPEND _project_info DESCRIPTION "GEOS - C++ port of the Java Topology Suite (JTS)")
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
  list(APPEND _project_info HOMEPAGE_URL "http://geos.osgeo.org")
endif()

project(GEOS VERSION "${_version_major}.${_version_minor}.${_version_patch}"
  LANGUAGES C CXX
  ${_project_info})

if("${_version_patch_word}" STREQUAL "")
  set(GEOS_VERSION_FULL "${GEOS_VERSION}")
else()
  set(GEOS_VERSION_FULL "${GEOS_VERSION}${_version_patch_word}")
endif()

unset(_version_major)
unset(_version_minor)
unset(_version_patch)
unset(_version_patch_word)

message(STATUS "GEOS: Version ${GEOS_VERSION_FULL}")
message(STATUS "GEOS: C API Version ${CAPI_VERSION}")
message(STATUS "GEOS: JTS port ${JTS_PORT}")

#-----------------------------------------------------------------------------
# C++ language version and compilation flags
#-----------------------------------------------------------------------------
message(STATUS "GEOS: Require C++${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-----------------------------------------------------------------------------
# Target geos_cxx_flags: common compilation flags
#-----------------------------------------------------------------------------
add_library(geos_cxx_flags INTERFACE)
target_compile_features(geos_cxx_flags INTERFACE cxx_std_11)

#-----------------------------------------------------------------------------
# Target geos_cxx_flags: common compilation flags
#-----------------------------------------------------------------------------
option(DISABLE_GEOS_INLINE "Disable inlining" OFF)
if(NOT DISABLE_GEOS_INLINE)
  target_compile_definitions(geos_cxx_flags INTERFACE GEOS_INLINE)
else()
  message(STATUS
  "GEOS: DISABLING inlining of small functions")
endif()

#-----------------------------------------------------------------------------
# Target geos_developer_cxx_flags: developer mode compilation flags
#-----------------------------------------------------------------------------
# Do NOT install this target for end-users!
add_library(geos_developer_cxx_flags INTERFACE)

if(GEOS_BUILD_DEVELOPER)
  message(STATUS "GEOS: Developer mode enabled")
endif()

# geos_cxx_flags inherits properties from geos_developer_cxx_flags when
# building as part of the GEOS repository or on explicit request for
# developer compilation mode, as GEOS contributor.
# The flags are intended only for GEOS itself and are not part of
# usage requirements needed by GEOS consumers.
if(GEOS_BUILD_DEVELOPER)
  target_link_libraries(geos_cxx_flags
    INTERFACE
      $<BUILD_INTERFACE:geos_developer_cxx_flags>)
endif()

target_compile_definitions(geos_cxx_flags
  INTERFACE
    USE_UNSTABLE_GEOS_CPP_API)

target_compile_definitions(geos_developer_cxx_flags
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
    $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_DEPRECATE>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)

target_compile_options(geos_developer_cxx_flags
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:-W4>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-pedantic -Wall -Wextra -Wno-long-long -Wcast-align -Wconversion -Wchar-subscripts -Wdouble-promotion -Wpointer-arith -Wformat -Wformat-security -Wshadow -Wuninitialized -Wunused-parameter -fno-common>
    $<$<CXX_COMPILER_ID:GNU>:-fno-implicit-inline-templates>)

# Disable TTMath ASM support on Windows due to build issues
if(WIN32)
  target_compile_definitions(geos_cxx_flags
    INTERFACE TTMATH_NOASM)
endif()

#-----------------------------------------------------------------------------
# Define a coverage build
#-----------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")

#-----------------------------------------------------------------------------
# Target geos: C++ API library
#-----------------------------------------------------------------------------
add_library(geos "")
target_link_libraries(geos PUBLIC geos_cxx_flags)
add_subdirectory(include)
add_subdirectory(src)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(geos
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:GEOS_DLL_EXPORT>)

  set_target_properties(geos PROPERTIES VERSION ${GEOS_VERSION})
  set_target_properties(geos PROPERTIES SOVERSION ${GEOS_VERSION})
endif()

#-----------------------------------------------------------------------------
# Target geos_c: C API library
#-----------------------------------------------------------------------------
add_library(geos_c "")
target_link_libraries(geos_c PRIVATE geos)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(geos_c
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:GEOS_DLL_EXPORT>)

  set_target_properties(geos_c PROPERTIES VERSION ${CAPI_VERSION})
  if(NOT WIN32)
    set_target_properties(geos_c PROPERTIES SOVERSION ${CAPI_VERSION_MAJOR})
  endif()
endif()

add_subdirectory(capi)

#-----------------------------------------------------------------------------
# Tests
#-----------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

#-----------------------------------------------------------------------------
# Benchmarks
#-----------------------------------------------------------------------------
add_subdirectory(benchmarks)

#-----------------------------------------------------------------------------
# Documentation/Examples
#-----------------------------------------------------------------------------
add_subdirectory(doc)

#-----------------------------------------------------------------------------
# Install and export targets - support 'make install' or equivalent
#-----------------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(GEOS_INSTALL_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake"
  VERSION ${GEOS_VERSION}
  COMPATIBILITY AnyNewerVersion)

if(NOT MSVC)
  configure_file(cmake/geos-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake"
    COPYONLY)
  list(APPEND GEOS_INSTALL_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake")
endif()

install(TARGETS geos geos_cxx_flags
  EXPORT geos-targets
  LIBRARY DESTINATION lib NAMELINK_SKIP
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

install(TARGETS geos_c
  EXPORT geos-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

install(EXPORT geos-targets
  FILE geos-targets.cmake
  NAMESPACE GEOS::
  DESTINATION lib/cmake/GEOS)

install(FILES
  ${GEOS_INSTALL_FILES}
  DESTINATION lib/cmake/GEOS)
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/geos"
  "${CMAKE_CURRENT_BINARY_DIR}/include/geos"
  DESTINATION include
  FILES_MATCHING PATTERN "*.h")
if(NOT DISABLE_GEOS_INLINE)
  install(DIRECTORY
    "${CMAKE_CURRENT_LIST_DIR}/include/geos"
    "${CMAKE_CURRENT_BINARY_DIR}/include/geos"
    DESTINATION include
    FILES_MATCHING PATTERN "*.inl")
endif()
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/capi/geos_c.h"
  DESTINATION include)

add_subdirectory(tools)

#-----------------------------------------------------------------------------
# Uninstall
#-----------------------------------------------------------------------------

configure_file("${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${PROJECT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake/cmake_uninstall.cmake")

#-----------------------------------------------------------------------------
# "make dist" workalike
#-----------------------------------------------------------------------------
get_property(_is_multi_config_generator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config_generator)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GEOS Computational Geometry Library")
  set(CPACK_PACKAGE_VENDOR "OSGeo")
  set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
  set(CPACK_SOURCE_GENERATOR "TBZ2")
  set(CPACK_PACKAGE_VERSION_MAJOR ${GEOS_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${GEOS_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${GEOS_VERSION_PATCH})
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "geos-${GEOS_VERSION_FULL}")

  set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git"
    "/autogen\\\\.sh"
    "/tools/ci"
    "/HOWTO_RELEASE"
    "/autom4te\\\\.cache"
    "\\\\.yml\$"
    "\\\\.deps"
    "/debian/"
    "/php/"
    "/.*build-.*/"
    ${PROJECT_BINARY_DIR}
    )

  # message(STATUS "GEOS: CPACK_SOURCE_PACKAGE_FILE_NAME: ${CPACK_SOURCE_PACKAGE_FILE_NAME}")
  # message(STATUS "GEOS: CPACK_SOURCE_IGNORE_FILES: ${CPACK_SOURCE_IGNORE_FILES}")
  # message(STATUS "GEOS: CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
  include(CPack)
  add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

  message(STATUS "GEOS: Configured 'dist' target")
endif()

#-----------------------------------------------------------------------------
# "make check" workalike
#-----------------------------------------------------------------------------

add_custom_target(check COMMAND ${CMAKE_BUILD_TOOL} test)

#-----------------------------------------------------------------------------
# "make distcheck" workalike
#-----------------------------------------------------------------------------
if(NOT _is_multi_config_generator)
  find_package(MakeDistCheck)
  AddMakeDistCheck()
  message(STATUS "GEOS: Configured 'distcheck' target")
endif()

unset(_is_multi_config_generator)
