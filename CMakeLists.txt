cmake_minimum_required(VERSION 3.21)
project(ChessAdventure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(CHESSADVENTURE_BUILD_TESTS "Build the unit test suite" ON)
option(CHESSADVENTURE_BUILD_MAP_EDITOR "Build the visual map editor tool" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(FetchContent)

# Some fetched dependencies (e.g. doctest) ship a CMakeLists.txt with an old
# cmake_minimum_required() that recent CMake refuses to configure without this.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- SDL3: expected to be installed on the system (apt/vcpkg/homebrew/etc). See
# README.md "Prerequisites" for install commands per platform. ---
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# --- Small header-only/vendorable dependencies: fetched at configure time so the
# build is reproducible without depending on distro package versions. ---
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# --- Engine-agnostic gameplay logic: board, pieces, upgrades, map data. No SDL
# dependency here on purpose, so it stays trivially unit-testable (see tests/). ---
add_library(chessadventure_core STATIC
    src/game/Board.cpp
    src/game/Tile.cpp
    src/maps/MapData.cpp
    src/maps/MapLoader.cpp
    src/pieces/MoveRules.cpp
    src/pieces/Piece.cpp
    src/pieces/PieceDatabase.cpp
    src/upgrades/SkillTree.cpp
)
target_include_directories(chessadventure_core PUBLIC src)
target_link_libraries(chessadventure_core PUBLIC nlohmann_json::nlohmann_json)
apply_project_warnings(chessadventure_core)

# --- SDL-backed rendering/app shell, built on top of chessadventure_core. ---
add_library(chessadventure_render STATIC
    src/render/SpriteSheet.cpp
    src/core/Application.cpp
)
target_link_libraries(chessadventure_render PUBLIC
    chessadventure_core
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)
apply_project_warnings(chessadventure_render)

add_executable(ChessAdventure src/main.cpp)
target_link_libraries(ChessAdventure PRIVATE chessadventure_render)
apply_project_warnings(ChessAdventure)

# Keep assets/ next to the built executable so it runs from the build directory
# without any install step, on every platform/generator.
add_custom_command(TARGET ChessAdventure POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:ChessAdventure>/assets"
    COMMENT "Copying assets/ next to ChessAdventure"
)

if(CHESSADVENTURE_BUILD_MAP_EDITOR)
    add_subdirectory(tools/map_editor)
endif()

if(CHESSADVENTURE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
