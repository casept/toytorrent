# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)

# Forbid in-source builds
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif()

project(toytorrent VERSION 0.1
                  DESCRIPTION "A toy torrent client"
                  LANGUAGES CXX)
add_executable(${PROJECT_NAME} src/main.cpp src/bencode_parser.cpp src/metainfo.cpp)
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# We want ISO C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -pedantic -Werror)
# Apparently cmake is so broken that C++17 doesn't actually get passed to the compiler without this
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

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

if(BUILD_TESTING)
    enable_testing()
    include(GoogleTest)
    add_executable(toytorrent_test src/test/main.cpp src/bencode_parser.cpp src/metainfo.cpp src/test/bencode_parser.cpp src/test/metainfo.cpp)
    gtest_add_tests(toytorrent_test "" AUTO)
    target_link_libraries(toytorrent_test PUBLIC gtest)
endif()