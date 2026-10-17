cmake_minimum_required(VERSION 3.16)

project(TinyTown
    VERSION 0.1.0
    DESCRIPTION "A tiny procedural world with simple autonomous agents, built with C++ and SDL2."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(TINYTOWN_BUILD_APP "Build the interactive SDL2 application" ON)
option(TINYTOWN_BUILD_TESTS "Build headless simulation tests" ON)

# ---------------------------------------------------------------------------
# Core simulation library: plain C++20, no graphics, no platform headers.
# ---------------------------------------------------------------------------
add_library(tinytown_core STATIC
    src/core/random.hpp
    src/core/types.hpp
    src/core/world.cpp
    src/core/pathfinding.cpp
    src/core/simulation.cpp
    src/core/camera.cpp
    src/app/session.cpp
    # The bitmap font and the language tables are pure data with no SDL or
    # OpenGL dependency, so they live in the headless library. That lets the
    # test binary check glyph coverage and translations without a window, and
    # lets Session own the active language.
    src/ui/font5x9.cpp
    src/ui/localization.cpp)

target_include_directories(tinytown_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(tinytown_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(tinytown_core PRIVATE /W4)
else()
    target_compile_options(tinytown_core PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Interactive application: SDL2 window + legacy OpenGL renderer.
# ---------------------------------------------------------------------------
if(TINYTOWN_BUILD_APP)
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)

    # Report which SDL2 was actually resolved. find_package(SDL2) will happily
    # accept any SDL2 it can find (a system package, a leftover SDK, or a
    # Windows-only archive picked up by a stray SDL2_DIR), so printing the
    # resolved location makes a wrong or missing SDK visible in the log instead
    # of failing later at link time.
    if(SDL2_DIR)
        message(STATUS "TinyTown: SDL2 found via SDL2_DIR=${SDL2_DIR}")
    elseif(SDL2_INCLUDE_DIRS)
        message(STATUS "TinyTown: SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
    endif()

    add_executable(tinytown
        src/app/main.cpp
        src/app/image_io.cpp
        src/render/renderer.cpp
        src/ui/hud.cpp)

    target_include_directories(tinytown PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(tinytown PRIVATE tinytown_core SDL2::SDL2 OpenGL::GL)

    # Optional PNG screenshot support. Without zlib the app still runs and
    # falls back to writing PPM files.
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        target_link_libraries(tinytown PRIVATE ZLIB::ZLIB)
        target_compile_definitions(tinytown PRIVATE TINYTOWN_HAVE_ZLIB=1)
        message(STATUS "TinyTown: PNG screenshot support enabled (zlib ${ZLIB_VERSION_STRING})")
    else()
        message(STATUS "TinyTown: zlib not found, screenshots will be written as PPM")
    endif()

    if(MSVC)
        target_compile_options(tinytown PRIVATE /W4)
    else()
        target_compile_options(tinytown PRIVATE -Wall -Wextra)
    endif()
endif()

# ---------------------------------------------------------------------------
# Headless tests: no SDL, no OpenGL, runnable on a build machine.
# ---------------------------------------------------------------------------
if(TINYTOWN_BUILD_TESTS)
    enable_testing()

    add_executable(tinytown_tests
        tests/test_main.cpp
        tests/test_world.cpp
        tests/test_pathfinding.cpp
        tests/test_simulation.cpp
        tests/test_camera.cpp
        tests/test_font.cpp
        tests/test_localization.cpp
        tests/test_session.cpp)

    target_include_directories(tinytown_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(tinytown_tests PRIVATE tinytown_core)

    if(MSVC)
        target_compile_options(tinytown_tests PRIVATE /W4)
    else()
        target_compile_options(tinytown_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME tinytown_core_tests COMMAND tinytown_tests)
endif()
