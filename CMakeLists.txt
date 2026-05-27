cmake_minimum_required(VERSION 3.19)
project(kalkulator LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(kalkulator
    WIN32 MACOSX_BUNDLE
    main.cpp
    calculator.cpp
    calculator.h
    calculator.ui
)

target_link_libraries(kalkulator
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS kalkulator
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET kalkulator
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
