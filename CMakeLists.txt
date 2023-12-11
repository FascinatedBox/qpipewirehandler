project(qpipewirehandler)
cmake_minimum_required(VERSION 3.5.0)

include(FindPkgConfig)

if(WITH_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")
endif()

if(WITH_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fprofile-arcs -ftest-coverage")
endif(WITH_COVERAGE)

pkg_check_modules(
    PIPEWIRE
    REQUIRED
        IMPORTED_TARGET
            libpipewire-0.3
)

find_package(
    Qt5
    REQUIRED
        Core
        Widgets
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(MOCUIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/.mocuic)
set(QPIPEWIREHANDLER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(demo)
