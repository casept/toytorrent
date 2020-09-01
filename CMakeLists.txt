# Semi-recent, I don't want to deal with outdated distro BS
cmake_minimum_required(VERSION 3.15)

# Forbid in-source builds
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif()

# Source files used both in the program and tests
list(APPEND SHARED_FILES "src/bencode_parser.cpp" "src/metainfo.cpp" "src/tracker.cpp" "src/peer.cpp")

project(toytorrent VERSION 0.1
                  DESCRIPTION "A toy torrent client"
                  LANGUAGES CXX)
add_executable(${PROJECT_NAME} src/main.cpp ${SHARED_FILES})


# We want ISO C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -pedantic -Werror)
# Apparently cmake is so broken that C++17 doesn't actually get passed to the compiler without this
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# CPR setup
find_package(CURL REQUIRED)
find_package(CPR REQUIRED)

set(BOTAN_DIR "cmake/")
find_package(BOTAN REQUIRED)

# Actually configure the target
# Don't move this around, position in the file was determined by trial and error
# Seriously, who though an imperative build system was a good idea, aaarrrrgh!!!
target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS} ${CPR_INCLUDE_DIRS} ${BOTAN_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES} ${CPR_LIBRARIES} ${BOTAN_LIBRARIES})
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

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

# Test setup
if(BUILD_TESTING)
    # Files only used for tests
    list(APPEND TEST_FILES "src/test/main.cpp" "src/test/bencode_parser.cpp" "src/test/metainfo.cpp" "src/test/tracker.cpp")
    enable_testing()
    include(GoogleTest)
    add_executable(${PROJECT_NAME}_test ${TEST_FILES} ${SHARED_FILES})
    gtest_add_tests(${PROJECT_NAME}_test "" AUTO)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CURL_INCLUDE_DIRS} ${CPR_INCLUDE_DIRS} ${BOTAN_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}_test PRIVATE gtest ${CURL_LIBRARIES} ${CPR_LIBRARIES} ${BOTAN_LIBRARIES})
    # Useful for integration testing of the nix package outside the nix sandbox (running tests in a container)
    if (INSTALL_TESTS)
        install(TARGETS toytorrent_test
            CONFIGURATIONS Release
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()