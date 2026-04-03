cmake_minimum_required(VERSION 3.15)
project(fitAide LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
set(NEEDED_QT_COMPONENTS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# SQLite3
find_package(SQLite3 REQUIRED)

# Sources
set(SOURCES
    src/fitAide.hpp
    src/main.cpp

    src/utils/errorhelper.cpp            src/utils/errorhelper.hpp

    src/database/database.cpp            src/database/database.hpp

    src/dialogs/exercisedialog.cpp       src/dialogs/exercisedialog.hpp
    src/dialogs/settingsdialog.cpp       src/dialogs/settingsdialog.hpp

    src/views/workoutview.cpp            src/views/workoutview.hpp

    src/widgets/exercisehead.cpp         src/widgets/exercisehead.hpp
    src/widgets/exerciseinstruct.cpp     src/widgets/exerciseinstruct.hpp
    src/widgets/exerciserecord.cpp       src/widgets/exerciserecord.hpp
    src/widgets/exerciserecordwarmup.cpp src/widgets/exerciserecord.cpp
    src/widgets/exerciserecordreps.cpp   src/widgets/exerciserecordreps.hpp
    src/widgets/exerciserecordset.cpp    src/widgets/exerciserecordset.hpp
    src/widgets/exercisestatus.cpp       src/widgets/exercisestatus.hpp
)

add_executable(fitAide ${SOURCES})

# Include directories
target_include_directories(fitAide PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SQLite3_INCLUDE_DIRS}
)

# Linking
target_link_libraries(fitAide PRIVATE
    ${NEEDED_QT_COMPONENTS}
    SQLite3::SQLite3
)

# === Development vs Release handling (no more folder checks) ===
# Use standard CMAKE_BUILD_TYPE (Debug / Release / RelWithDebInfo etc.)
# DEVELOPMENT_MODE define is set automatically for Debug builds

target_compile_definitions(fitAide PRIVATE
    $<$<CONFIG:Debug>:DEVELOPMENT_MODE>
)

# Compiler options (cross-platform friendly)
target_compile_options(fitAide PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O2>
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Release>:/O2>
    >
)

# Strip symbols in Release (portable-ish)
target_link_options(fitAide PRIVATE
    $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,$<CONFIG:Release>>:-s>
)

# Set executable name
set_target_properties(fitAide PROPERTIES
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,fitAideDev,fitAide>"
)

# Optional: Windows-specific to hide console in Release
if(WIN32)
    target_link_options(fitAide PRIVATE $<$<CONFIG:Release>:-mwindows>)
endif()

add_custom_target(run
    COMMAND "$<TARGET_FILE:fitAide>"
    DEPENDS fitAide
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    USES_TERMINAL
    VERBATIM
    COMMENT "Running fitAide $<IF:$<CONFIG:Debug>,Development,Release> build..."
)