project(qpipewirehandler)
cmake_minimum_required(VERSION 3.5.0)

include(FindPkgConfig)
find_package(
    Qt5
    COMPONENTS
        Widgets
    REQUIRED
)
pkg_check_modules(
    PIPEWIRE
        IMPORTED_TARGET
            libpipewire-0.3
    REQUIRED
)

if(WITH_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")
endif()

set(CMAKE_AUTOMOC ON)
set(MOCUIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/.mocuic)

add_executable(
    qpipewirehandler
        demo/demo.cpp
        demo/democontroller.cpp
        demo/demoviewer.cpp
        src/qpipewirehandler.cpp
)

target_include_directories(
    qpipewirehandler
    PRIVATE
        src/
)

set_target_properties(
    qpipewirehandler
    PROPERTIES
        AUTOGEN_BUILD_DIR
            ${MOCUIC_DIR}
)

target_link_libraries(
    qpipewirehandler
        PkgConfig::PIPEWIRE
        Qt5::Core
        Qt5::Widgets
)
