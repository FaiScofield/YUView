cmake_minimum_required(VERSION 3.16)

project(YUView VERSION 3.0.0 LANGUAGES C CXX)

message(STATUS "Project ${PROJECT_NAME} - v${PROJECT_VERSION} - ${CMAKE_BUILD_TYPE}")
message(STATUS "Get compiler info below:")
if(MSVC)
    message(STATUS "\tMSVC version: ${MSVC_VERSION}")
    message(STATUS "\tMSVC generator: ${CMAKE_GENERATOR}")
endif()
message(STATUS "\tC compiler: ${CMAKE_C_COMPILER} - ${CMAKE_C_COMPILER_ID} - ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "\tC++ compiler: ${CMAKE_CXX_COMPILER} - ${CMAKE_CXX_COMPILER_ID} - ${CMAKE_CXX_COMPILER_VERSION}")

# Options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # include the  AUTOUIC generated header dir

option(ENABLE_CONSOLE "Enable Subsystem Console" OFF)
option(UNITTESTS "Build unit tests" OFF)

# Git version config
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
    message(STATUS "Git found successfully!")
    message(STATUS "\tGit tags: ${GIT_DESCRIBE_TAGS}")
    message(STATUS "\tGit hash: ${GIT_COMMIT_HASH}")
else()
    set(GIT_DESCRIBE_TAGS "0")
    set(GIT_COMMIT_HASH "0")
    message(WARNING "Git not found! Git version tags and hash will be set to 0.")
endif()

# QT configuration
list(APPEND CMAKE_PREFIX_PATH ${QT_PATH}) # Qt Kit Dir
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Xml Concurrent Network)
if(Qt5_FOUND)
    message(STATUS "Qt5 found successfully!")
    message(STATUS "\tQt5 version: ${Qt5_VERSION}")
    message(STATUS "\tQt5 components: ${Qt5_LIBS}")
else()
    message(FATAL_ERROR "Qt5 not found!")
endif()

# Add definitions
add_definitions(-DYUVIEW_VERSION="${PROJECT_VERSION}")
add_definitions(-DYUVIEW_TAGS="${GIT_DESCRIBE_TAGS}")
add_definitions(-DYUVIEW_HASH="${GIT_COMMIT_HASH}")
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Install
set(RUNTIME_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/release/v${PROJECT_VERSION}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RUNTIME_OUTPUT_DIR "${RUNTIME_OUTPUT_DIR}d")
endif()
message(STATUS "Set install dir: ${RUNTIME_OUTPUT_DIR}")

# Add subdirectories
add_subdirectory(YUViewLib)
add_subdirectory(YUViewApp)

# Handle unit tests if enabled
if(UNITTESTS)
    add_subdirectory(submodules/googletest-qmake)
    add_subdirectory(YUViewUnitTest)
endif()

