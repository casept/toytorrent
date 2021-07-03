# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)

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

# 3rd-party deps
find_package(Botan2 2.0 REQUIRED)
find_package(CPR REQUIRED)
find_package(fmt REQUIRED)
# System deps
find_package(Threads REQUIRED QUIET)

# Compile options for debugging
list(
  APPEND
  DEBUG_COMPILE_OPTS
  -Wall
  -Wextra
  -Wpedantic
  -O0
  -g
  -fdiagnostics-color=always
  -fsanitize=address
  -fsanitize=undefined
  -fno-omit-frame-pointer)
list(APPEND DEBUG_LD_OPTS -fno-omit-frame-pointer -fsanitize=address
     -fsanitize=undefined)

# Core library
target_compile_options(lib${PROJECT_NAME} PRIVATE ${DEBUG_COMPILE_OPTS})
target_link_libraries(lib${PROJECT_NAME} PRIVATE cpr Botan2::Botan2 fmt::fmt)

# Executable
target_compile_options(${PROJECT_NAME} PRIVATE ${DEBUG_COMPILE_OPTS})
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
  target_compile_options(${PROJECT_NAME}_test PRIVATE ${DEBUG_COMPILE_OPTS})
  target_link_libraries(
    ${PROJECT_NAME}_test
    PRIVATE lib${PROJECT_NAME} gtest Boost::headers Boost::system
            Boost::filesystem Threads::Threads)
  target_link_options(${PROJECT_NAME}_test PRIVATE ${DEBUG_LD_OPTS})
  # Useful to run tests in a different environment than the build (for example,
  # outside the Nix build sandbox so we can talk to a real tracker)
  if(INSTALL_TESTS)
    install(
      TARGETS ${PROJECT_NAME}_test
      CONFIGURATIONS Release
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()
