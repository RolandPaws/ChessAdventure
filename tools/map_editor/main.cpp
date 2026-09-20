#include <iostream>

#include "EditorApp.h"

int main(int /*argc*/, char** /*argv*/) {
    chessadventure::editor::EditorApp app;
    if (!app.Init()) {
        std::cerr << "Failed to initialize the map editor.\n";
        return 1;
    }
    return app.Run();
}
