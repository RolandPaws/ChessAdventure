#include <filesystem>
#include <iostream>

#include "core/Application.h"

// Resolves the assets/ directory relative to this executable, so `./ChessAdventure`
// works both from the build directory and from an installed layout.
static std::filesystem::path FindAssetsRoot() {
    namespace fs = std::filesystem;
    fs::path candidate = fs::current_path();
    for (int i = 0; i < 6; ++i) {
        if (fs::exists(candidate / "assets" / "data" / "pieces.json"))
            return candidate;
        candidate = candidate.parent_path();
    }
    return fs::current_path();  // fall back; LoadContent will report the failure clearly
}

int main(int /*argc*/, char** /*argv*/) {
    const std::filesystem::path assets_root = FindAssetsRoot();

    chessadventure::Application app(1024, 1024);
    if (!app.Init()) {
        std::cerr << "Failed to initialize application.\n";
        return 1;
    }

    const auto piece_db_path = (assets_root / "assets" / "data" / "pieces.json").string();
    const auto map_path = (assets_root / "assets" / "maps" / "example_map.json").string();
    if (!app.LoadContent(piece_db_path, map_path)) {
        std::cerr << "Failed to load content from " << assets_root << "\n";
        return 1;
    }

    return app.Run();
}
