cmake_minimum_required(VERSION 3.20)
project(fitAide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src/*.hpp")

add_executable(fitAide ${SOURCES} ${HEADERS})

target_link_libraries(fitAide PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
    PkgConfig::SQLITE3
)

target_include_directories(fitAide PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)

option(BUILD_DEVELOPMENT_MODE "Build in development mode" ON)
if(BUILD_DEVELOPMENT_MODE)
    target_compile_definitions(fitAide PRIVATE DEVELOPMENT_MODE)
    set_target_properties(fitAide PROPERTIES OUTPUT_NAME "fitAideDev")
    target_compile_options(fitAide PRIVATE -g -Wall -Wextra -DDEVELOPMENT_MODE)
    target_link_options(fitAide PRIVATE -s)
    #if(WIN32)
    #    target_link_options(fitAide PRIVATE -mwindows)
    #endif()
else()
    target_compile_options(fitAide PRIVATE -O2)
    target_link_options(fitAide PRIVATE -s)
    # if(WIN32)
    #    target_link_options(fitAide PRIVATE -mwindows)
    # endif()
endif()

# Custom targets to run the executables
add_custom_target(run_dev
    COMMAND "$<TARGET_FILE:fitAide>"
    DEPENDS fitAide
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    USES_TERMINAL
    VERBATIM
    COMMENT "Running development executable..."
)
add_custom_target(run_release
    COMMAND "$<TARGET_FILE:fitAide>"
    DEPENDS fitAide
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    USES_TERMINAL
    VERBATIM
    COMMENT "Running release executable..."
)