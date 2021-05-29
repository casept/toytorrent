# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Fix stuff that's broken by default
include(ForbidInSource)
include(SetBuildType)

# Source files used both in the program and tests
list(
  APPEND
  SHARED_FILES
  "src/bencode_parser.cpp"
  "src/metainfo.cpp"
  "src/tracker.cpp"
  "src/peer.cpp"
  "src/torrent.cpp"
  "src/smolsocket.cpp")
# Source files only used in the main program, but not tests
list(APPEND EXE_FILES "src/main.cpp")
# Source files only used for tests
list(APPEND TEST_SOURCE_FILES "src/test/main.cpp" "src/test/bencode_parser.cpp"
     "src/test/metainfo.cpp" "src/test/tracker.cpp")

project(
  toytorrent
  VERSION 0.1
  DESCRIPTION "A toy torrent client"
  LANGUAGES CXX)
add_executable(${PROJECT_NAME} ${EXE_FILES} ${SHARED_FILES})

# We want ISO C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# 3rd-party deps
find_package(Botan2 2.0 REQUIRED)
find_package(CPR REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr Botan2::Botan2 fmt::fmt)

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Test setup
if(BUILD_TESTING)
  # In order for the test binary to run out-of-tree we need to include test
  # assets into it
  set(CMakeRC_DIR "cmake/")
  find_package(CMakeRC REQUIRED)
  list(APPEND TEST_DATA_FILES "src/test/testdata/lubuntu-16.04.torrent")
  cmrc_add_resource_library(test_resources WHENCE "src/test/testdata/"
                            ${TEST_DATA_FILES})

  enable_testing()
  include(GoogleTest)
  add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_FILES} ${SHARED_FILES})

  gtest_discover_tests(${PROJECT_NAME}_test "" AUTO)
  target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest test_resources cpr
                                                     Botan2::Botan2 fmt::fmt)

  # Useful to run tests in a different environment than the build (for example,
  # outside the Nix build sandbox so we can talk to a real tracker)
  if(INSTALL_TESTS)
    install(
      TARGETS ${PROJECT_NAME}_test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()
