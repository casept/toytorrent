# Earlier versions are not tested or supported
cmake_minimum_required(VERSION 3.19)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Fix stuff that's broken by default
include(ForbidInSource)
include(SetBuildType)

project(
  toytorrent
  VERSION 0.1
  DESCRIPTION "A toy torrent client"
  LANGUAGES CXX)
add_executable(${PROJECT_NAME} "src/main.cpp")
add_library(
  lib${PROJECT_NAME} STATIC
  "src/bencode.cpp"
  "src/metainfo.cpp"
  "src/tracker.cpp"
  "src/peer.cpp"
  "src/peer_message.cpp"
  "src/piece.cpp"
  "src/torrent.cpp"
  "src/byteorder.cpp"
  "src/smolsocket.cpp"
  "src/log.cpp")

# We want ISO C++20
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_features(lib${PROJECT_NAME} PUBLIC cxx_std_20)
target_precompile_headers(
  lib${PROJECT_NAME}
  PRIVATE
  <algorithm>
  <array>
  <cstdint>
  <deque>
  <string>
  <string_view>
  <optional>
  <functional>
  <utility>
  <vector>
  <fstream>
  <stdexcept>
  <filesystem>
  <iostream>
  <istream>
  <iterator>
  <memory>
  <tuple>
  <cpr/cpr.h>
  <fmt/core.h>
  <fmt/format.h>
  <botan-2/botan/auto_rng.h>
  <botan-2/botan/hash.h>
  <botan-2/botan/hex.h>)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM lib${PROJECT_NAME})

# 3rd-party deps
find_package(Botan2 2.0 REQUIRED)
find_package(CPR REQUIRED)
find_package(fmt REQUIRED)
# System deps
find_package(Threads REQUIRED QUIET)

if(CMAKE_BUILD_TYPE EQUAL "Debug")
  list(APPEND SHARED_COMPILE_OPTS -O0 -g)
endif()

option(ENABLE_SANITIZERS "Enable address and UB sanitizers." FALSE)
if(ENABLE_SANITIZERS)
  list(APPEND SHARED_COMPILE_OPTS -fsanitize=address -fsanitize=undefined
       -fno-omit-frame-pointer)
  list(APPEND DEBUG_LD_OPTS -fno-omit-frame-pointer -fsanitize=address
       -fsanitize=undefined)
endif()

option(FORCE_COLORED_OUTPUT
       "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if(${FORCE_COLORED_OUTPUT})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND SHARED_COMPILE_OPTS -fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND SHARED_COMPILE_OPTS -fcolor-diagnostics)
  endif()
endif()

option(ENABLE_WARNINGS "Enable strict compiler warnings (GNU/Clang only)."
       FALSE)
if(${ENABLE_WARNINGS})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND SHARED_COMPILE_OPTS -Wall -Wextra -Wpedantic)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND SHARED_COMPILE_OPTS -Wall -Wextra -Wpedantic)
  endif()
endif()

# Core library
target_compile_options(lib${PROJECT_NAME} PRIVATE ${SHARED_COMPILE_OPTS})
target_link_libraries(lib${PROJECT_NAME} PRIVATE cpr Botan2::Botan2 fmt::fmt)

# Executable
target_compile_options(${PROJECT_NAME} PRIVATE ${SHARED_COMPILE_OPTS})
target_link_options(${PROJECT_NAME} PRIVATE ${DEBUG_LD_OPTS})
target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})

# Hack to force cmake to add system (libstdc++) header path to
# compile_commands.json. This also adds a lot of junk, but as long as it doesn't
# slow clangd too much I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Test setup
option(BUILD_TESTING "Build test framework" FALSE)
if(BUILD_TESTING)
  # The tests use boost
  find_package(
    Boost
    COMPONENTS system filesystem
    REQUIRED)
endif()

# Hack to force cmake to add system (libstdc++) header path to
# compile_commands.json. This also adds a lot of junk, but as long as it doesn't
# slow clangd too much I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

if(BUILD_TESTING)
  enable_testing()
  include(GoogleTest)
  add_executable(
    ${PROJECT_NAME}_test
    "src/test/main.cpp"
    "src/test/helpers.cpp"
    "src/test/bencode.cpp"
    "src/test/metainfo.cpp"
    "src/test/tracker.cpp"
    "src/test/peer.cpp"
    "src/test/torrent.cpp")
  gtest_discover_tests(${PROJECT_NAME}_test "" AUTO)
  target_compile_options(${PROJECT_NAME}_test PRIVATE ${SHARED_COMPILE_OPTS})
  target_link_libraries(
    ${PROJECT_NAME}_test
    PRIVATE lib${PROJECT_NAME} gtest Boost::headers Boost::system
            Boost::filesystem Threads::Threads)
  target_link_options(${PROJECT_NAME}_test PRIVATE ${DEBUG_LD_OPTS})
  target_precompile_headers(
    ${PROJECT_NAME}_test
    PRIVATE
    <cstdint>
    <chrono>
    <string>
    <string_view>
    <vector>
    <boost/process.hpp>
    <gtest/gtest.h>
    <fmt/core.h>)
endif()
