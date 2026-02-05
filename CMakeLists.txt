cmake_minimum_required(VERSION 3.16)

project(YUView VERSION 2.14.0 LANGUAGES C CXX)

message(STATUS "project ${PROJECT_NAME} - v${PROJECT_VERSION} - ${CMAKE_BUILD_TYPE}")
if(MSVC)
    message(STATUS "\tMSVC version: ${MSVC_VERSION}")
    message(STATUS "\tMSVC generator: ${CMAKE_GENERATOR}")
endif()
message(STATUS "C compiler: ${CMAKE_C_COMPILER} - ${CMAKE_C_COMPILER_ID} - ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER} - ${CMAKE_CXX_COMPILER_ID} - ${CMAKE_CXX_COMPILER_VERSION}")


# Set up AUTOMOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# include(GNUInstallDirs)
list(APPEND CMAKE_PREFIX_PATH ${QT_PATH}) # Qt Kit Dir
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Xml Concurrent Network)
if(Qt5_FOUND)
    message(STATUS "\tQt5 found successfully!")
    message(STATUS "\tQt5 version: ${Qt5_VERSION}")
    message(STATUS "\tQt5 components: ${Qt5_LIBS}")
else()
    message(FATAL_ERROR "\tQt5 not found!")
endif()

# Get git hash and version
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_DESCRIBE_TAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT GIT_DESCRIBE_TAGS)
        set(GIT_DESCRIBE_TAGS "0")
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT GIT_COMMIT_HASH)
        set(GIT_COMMIT_HASH "0")
    endif()
else()
    set(GIT_DESCRIBE_TAGS "0")
    set(GIT_COMMIT_HASH "0")
endif()

add_definitions(-DYUVIEW_VERSION="${PROJECT_VERSION}")
add_definitions(-DYUVIEW_TAGS="${GIT_DESCRIBE_TAGS}")
add_definitions(-DYUVIEW_HASH="${GIT_COMMIT_HASH}")
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Add subdirectories
add_subdirectory(YUViewLib)
add_subdirectory(YUViewApp)

# Handle unit tests if enabled
option(UNITTESTS "Build unit tests" OFF)
if(UNITTESTS)
    add_subdirectory(submodules/googletest-qmake)
    add_subdirectory(YUViewUnitTest)
endif()

