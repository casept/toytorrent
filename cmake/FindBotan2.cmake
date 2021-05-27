cmake_minimum_required(VERSION 3.15)

# Find using pkgconfig
find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(PC_Botan2 QUIET botan-2)

set(Botan2_FOUND ${PC_Botan2_FOUND})
# mark_as_advanced(Botan2_FOUND)

set(Botan2_VERSION ${PC_Botan2_VERSION})
mark_as_advanced(Botan2_VERSION)

# Find header path
find_path(
  Botan2_INCLUDE_DIR
  NAMES botan/botan.h
  HINTS ${PC_Botan2_INCLUDE_DIRS})
mark_as_advanced(Botan2_INCLUDE_DIR)

# Find lib path
find_library(
  Botan2_LIBRARIES
  NAMES botan-2
  HINTS ${PC_Botan2_LIBRARY_DIRS})
mark_as_advanced(Botan2_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Botan2
  FOUND_VAR Botan2_FOUND
  REQUIRED_VARS Botan2_INCLUDE_DIR Botan2_LIBRARIES
  VERSION_VAR Botan2_VERSION)

if(Botan2_FOUND)
  set(Botan2_INCLUDE_DIRS ${Botan2_INCLUDE_DIR})

  add_library(Botan2::Botan2 INTERFACE IMPORTED)
  target_include_directories(Botan2::Botan2 INTERFACE ${Botan2_INCLUDE_DIRS})
  target_link_libraries(Botan2::Botan2 INTERFACE ${Botan2_LIBRARIES})
endif()
