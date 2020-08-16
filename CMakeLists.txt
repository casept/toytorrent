# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)
project(toytorrent VERSION 0.1
                  DESCRIPTION "A toy torrent client"
                  LANGUAGES CPP)
add_executable(${PROJECT_NAME} src/main.cpp src/bencoding.cpp)

# We want ISO C++17
set(CMAKE_CPP_STANDARD 17)
set(CMAKE_CPP_STANDARD_REQUIRED ON)
set(CMAKE_CPP_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Boilerplate for setting the build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Clang-tidy setup
option(CLANG_TIDY_FIX "Perform fixes for Clang-Tidy" OFF)
find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)
if(CLANG_TIDY_EXE)
    if(CLANG_TIDY_FIX)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
    else()
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endif()
if (CLANG_TIDY_FIX AND NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "Could NOT find clang-tidy")
endif()
