cmake_minimum_required(VERSION 3.21)
# C is needed alongside CXX because SDL3/SDL3_image/SDL3_ttf are C projects;
# when they aren't found on the system, they get pulled in and built from
# source below (see the SDL3 fetch block).
project(ChessAdventure LANGUAGES C CXX)

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

# --- SDL3: prefer a system install (apt/vcpkg/homebrew/etc, see README.md
# "Prerequisites") since it configures in seconds; fall back to building from
# source when it's missing -- e.g. Ubuntu's apt repos didn't carry
# libsdl3-dev until 24.10, so the "ubuntu-latest" GitHub Actions image (24.04)
# needs this path. Both paths produce the same SDL3::SDL3 / SDL3_image::SDL3_image
# / SDL3_ttf::SDL3_ttf targets used below.
find_package(SDL3 3.2 QUIET CONFIG)
find_package(SDL3_image 3.2 QUIET CONFIG)
find_package(SDL3_ttf 3.2 QUIET CONFIG)

if(NOT SDL3_FOUND OR NOT SDL3_image_FOUND OR NOT SDL3_ttf_FOUND)
    message(STATUS "System SDL3/SDL3_image/SDL3_ttf not found -- fetching and building from source")

    # Keep all three built the same way (static) so their generated CMake
    # targets agree -- a shared SDL3_image linked against a static SDL3 (or
    # vice versa) fails at CMake generate time with an INTERFACE_SDL3_SHARED
    # property mismatch.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

    if(NOT SDL3_FOUND)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.2.30
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3)
    endif()

    if(NOT SDL3_image_FOUND)
        # We only ever load PNG/JPG (via the bundled stb_image backend, which
        # needs no external codec library) plus a couple of trivial built-in
        # formats. Skip AVIF/JXL/TIFF/WEBP -- those need vendored submodules
        # (dav1d, aom, libjxl, libtiff, libwebp, ...) that are heavy to fetch
        # and build and that this game doesn't use. GIT_SUBMODULES "" stops
        # FetchContent from cloning *any* of SDL3_image's optional codec
        # submodules regardless of these flags.
        set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
        set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
        set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
        set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            SDL3_image
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
            GIT_TAG release-3.2.6
            GIT_SHALLOW TRUE
            GIT_SUBMODULES ""
        )
        FetchContent_MakeAvailable(SDL3_image)
    endif()

    if(NOT SDL3_ttf_FOUND)
        # Non-vendored (the Linux default) requires system freetype -- see
        # README.md "Prerequisites" / the CI workflow for the apt package.
        FetchContent_Declare(
            SDL3_ttf
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
            GIT_TAG release-3.2.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SDL3_ttf)
    endif()
endif()

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
