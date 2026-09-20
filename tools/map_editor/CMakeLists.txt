# Dear ImGui has no CMakeLists.txt of its own upstream -- it's meant to be dropped
# into a host build system directly, so we build the handful of source files we
# need (core + the SDL3/SDLRenderer3 backends) as our own static lib target.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui SYSTEM PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

add_executable(chessadventure_map_editor
    main.cpp
    EditorApp.cpp
)
target_link_libraries(chessadventure_map_editor PRIVATE chessadventure_core imgui SDL3::SDL3)
apply_project_warnings(chessadventure_map_editor)

add_custom_command(TARGET chessadventure_map_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:chessadventure_map_editor>/assets"
    COMMENT "Copying assets/ next to chessadventure_map_editor"
)
