FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(doctest)

add_executable(chessadventure_tests
    test_main.cpp
    test_move_rules.cpp
    test_board.cpp
    test_skill_tree.cpp
    test_map_loader.cpp
)
target_link_libraries(chessadventure_tests PRIVATE chessadventure_core doctest::doctest)
apply_project_warnings(chessadventure_tests)

# Tests load the real shipped data files (assets/data/*.json, assets/maps/*.json)
# rather than a separate fixture copy, so a broken data file fails the test suite
# instead of only surfacing at runtime.
target_compile_definitions(chessadventure_tests PRIVATE
    CHESSADVENTURE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

add_test(NAME chessadventure_tests COMMAND chessadventure_tests)
