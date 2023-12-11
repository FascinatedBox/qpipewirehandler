include_directories(../src)

add_executable(
    qpipewirehandler
        demo.cpp
        demoviewer.cpp
        democontroller.cpp
        ../src/qpipewirehandler.cpp
)

target_link_libraries(
    qpipewirehandler
        PkgConfig::PIPEWIRE
        Qt5::Core
        Qt5::Widgets
)

set_target_properties(
    qpipewirehandler
    PROPERTIES
        AUTOGEN_BUILD_DIR
            ${MOCUIC_DIR}
        RUNTIME_OUTPUT_DIRECTORY
            ${QPIPEWIREHANDLER_BINARY_DIR}
)

install(
    TARGETS
        qpipewirehandler
    DESTINATION
        bin
)
