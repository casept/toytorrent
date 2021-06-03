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
  "src/bencode.cpp"
  "src/metainfo.cpp"
  "src/tracker.cpp"
  "src/peer.cpp"
  "src/torrent.cpp"
  "src/smolsocket.cpp"
  "src/log.cpp")
# Source files only used in the main program, but not tests
list(APPEND EXE_FILES "src/main.cpp")
# Source files only used for tests
list(
  APPEND
  TEST_SOURCE_FILES
  "src/test/main.cpp"
  "src/test/helpers.cpp"
  "src/test/bencode.cpp"
  "src/test/metainfo.cpp"
  "src/test/tracker.cpp")

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
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -O0 -g
                                               -fdiagnostics-color=always)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Hack to force cmake to add system (libstdc++) header path to
# compile_commands.json. This also adds a lot of junk, but as long as it doesn't
# slow clangd too much I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# 3rd-party deps
find_package(Botan2 2.0 REQUIRED)
find_package(CPR REQUIRED)
find_package(fmt REQUIRED)
# System deps
find_package(Threads REQUIRED QUIET)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr Botan2::Botan2 fmt::fmt)

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Test setup
if(BUILD_TESTING)
  # The tests use boost
  find_package(
    Boost
    COMPONENTS system filesystem
    REQUIRED)
  # Hack to force cmake to add system (libstdc++) header path to
  # compile_commands.json. This also adds a lot of junk, but as long as it
  # doesn't slow clangd too much I'll deal.
  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
        ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  endif()

  enable_testing()
  include(GoogleTest)
  add_executable(${PROJECT_NAME}_test ${TEST_SOURCE_FILES} ${SHARED_FILES})

  gtest_discover_tests(${PROJECT_NAME}_test "" AUTO)
  target_compile_options(
    ${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic
                                 -fdiagnostics-color=always)
  target_link_libraries(
    ${PROJECT_NAME}_test
    PRIVATE gtest
            cpr
            Boost::headers
            Boost::system
            Boost::filesystem
            Botan2::Botan2
            fmt::fmt
            Threads::Threads)

  # Useful to run tests in a different environment than the build (for example,
  # outside the Nix build sandbox so we can talk to a real tracker)
  if(INSTALL_TESTS)
    install(
      TARGETS ${PROJECT_NAME}_test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()
