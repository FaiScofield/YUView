cmake_minimum_required(VERSION 3.16)
project(googletest-qmake VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core)
endif()

add_subdirectory(gtest)
add_subdirectory(gtest_main)