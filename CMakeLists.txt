cmake_minimum_required(VERSION 3.19)
project(YouTube-Download-Manager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(YouTube-Download-Manager
    WIN32 MACOSX_BUNDLE
    main.cpp
    widget.cpp
    widget.h
    widget.ui


)

target_link_libraries(YouTube-Download-Manager
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

install(TARGETS YouTube-Download-Manager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET YouTube-Download-Manager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
