#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "game/Board.h"
#include "game/Position.h"
#include "pieces/PieceDatabase.h"

namespace chessadventure {

// Owns the SDL lifecycle (window/renderer), the main loop, and the currently loaded
// board. This is the game's entry point wiring, not gameplay logic itself — Board,
// Piece, and MoveRules stay engine-agnostic and unit-testable.
class Application {
public:
    Application(int window_width, int window_height);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Returns false if SDL/window/renderer setup failed.
    bool Init();

    // Loads piece templates + a map, builds the playable Board. Must be called
    // after Init(). Returns false if either asset file fails to load.
    bool LoadContent(const std::string& piece_db_path, const std::string& map_path);

    // Runs the loop until the window is closed. Returns a process exit code.
    int Run();

private:
    void HandleEvent(const SDL_Event& event);
    void Update(float delta_seconds);
    void Render();

    Position ScreenToBoard(float screen_x, float screen_y) const;
    void DrawBoard();
    void DrawPieces();
    void DrawHighlights();

    int window_width_;
    int window_height_;
    int tile_size_ = 64;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    bool running_ = false;

    std::unique_ptr<PieceDatabase> piece_db_;
    std::unique_ptr<Board> board_;

    std::optional<Position> selected_;
    std::vector<Position> legal_moves_for_selected_;
};

}  // namespace chessadventure
