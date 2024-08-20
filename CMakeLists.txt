# You only need one for the whole project.
cmake_minimum_required (VERSION 3.21)

# Don't set() the project name first and then pass it to project()
project(nk-theme-editor VERSION 0.1)

# Set C/C++17 standard to ensure clang support while taking advantage of newer langauge functionality.
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# Set the correct startup project in case Visual Studio is to be used
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

#FetchContent is better than ExternalContent if the external lib uses CMake
include(FetchContent)

option(NO_EXTERNAL_LIBS "Disable fetching external libraries before building" OFF)

if(NO_EXTERNAL_LIBS)
    find_package(SDL2 REQUIRED)
endif()

add_subdirectory (src)
