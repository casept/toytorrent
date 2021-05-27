# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)

# Forbid in-source builds
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(
    FATAL_ERROR
      "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif()

# Source files used both in the program and tests
list(APPEND SHARED_FILES "src/bencode_parser.cpp" "src/metainfo.cpp"
     "src/tracker.cpp" "src/peer.cpp")

project(
  toytorrent
  VERSION 0.1
  DESCRIPTION "A toy torrent client"
  LANGUAGES CXX)
# Source files only used in the main program, but not tests
list(APPEND EXE_FILES "src/main.cpp")
add_executable(${PROJECT_NAME} ${EXE_FILES} ${SHARED_FILES})

# We want ISO C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# TODO: Add -Werror once bin2cpp no longer adds unneeded semicolons
add_compile_options(-Wall -Wextra -Wpedantic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# 3rd-party deps
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(Botan2 2.0 REQUIRED)
find_package(CPR REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE cpr Botan2::Botan2)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr Botan2::Botan2)

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Boilerplate for setting the build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Test setup
if(BUILD_TESTING)
  # In order for the test binary to run out-of-tree we need to include test
  # assets into it
  set(CMakeRC_DIR "cmake/")
  find_package(CMakeRC REQUIRED)
  list(APPEND TEST_DATA_FILES "src/test/testdata/lubuntu-16.04.torrent")
  cmrc_add_resource_library(test_resources WHENCE "src/test/testdata/"
                            ${TEST_DATA_FILES})

  # Source files only used for tests
  list(APPEND TEST_SOURCE_FILES "src/test/main.cpp"
       "src/test/bencode_parser.cpp" "src/test/metainfo.cpp"
       "src/test/tracker.cpp")

  enable_testing()
  include(GoogleTest)
  add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_FILES} ${SHARED_FILES})

  gtest_discover_tests(${PROJECT_NAME}_test "" AUTO)
  target_include_directories(${PROJECT_NAME}_test PRIVATE cpr Botan2::Botan2)
  target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest test_resources cpr
                                                     Botan2::Botan2)

  # Useful to run tests in a different environment than the build (for example,
  # outside the Nix build sandbox so we can talk to a real tracker)
  if(INSTALL_TESTS)
    install(
      TARGETS ${PROJECT_NAME}_test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()
