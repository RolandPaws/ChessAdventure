#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>

#include "game/Position.h"
#include "maps/MapData.h"
#include "pieces/PieceDatabase.h"

namespace chessadventure::editor {

// Which kind of thing a canvas click currently paints. This is deliberately a flat
// mode switch (not a plugin/tool interface) -- see docs/ART_PIPELINE.md and
// ROADMAP.md for why the editor stays this simple until the paint tools it needs
// are actually known.
enum class PaintMode {
    kTile,
    kPlacePiece,
    kErase,
};

// A minimal, functional visual map editor: paints tiles and piece spawns onto a
// grid and reads/writes the same MapData JSON format the game loads (see
// maps/MapLoader.h). This is the "beginning" of the map tooling -- undo, layers,
// and multi-select are intentionally out of scope for now (see ROADMAP.md).
class EditorApp {
public:
    EditorApp();
    ~EditorApp();

    EditorApp(const EditorApp&) = delete;
    EditorApp& operator=(const EditorApp&) = delete;

    bool Init();
    int Run();

private:
    void HandleEvent(const SDL_Event& event);
    void DrawUi();
    void DrawCanvas();

    // Screen-space rect where the map grid is drawn, recomputed each frame from the
    // current window size minus the side panel.
    SDL_FRect CanvasRect() const;
    Position CanvasPixelToTile(float x, float y) const;

    void NewMap(int width, int height);
    void LoadMap(const std::string& path);
    void SaveMap(const std::string& path);

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = false;

    PieceDatabase piece_db_;
    MapData map_ = MapData::MakeEmpty(8, 8);

    PaintMode paint_mode_ = PaintMode::kTile;
    TileType selected_tile_type_ = TileType::kFloor;
    std::string selected_piece_id_;
    bool placing_for_enemy_team_ = false;

    float tile_size_ = 48.0f;
    std::string status_message_;

    std::array<char, 256> path_buffer_{"assets/maps/example_map.json"};
    std::array<char, 128> name_buffer_{"Untitled Map"};
    std::array<char, 1024> intro_buffer_{};
    std::array<char, 1024> on_victory_buffer_{};
    int victory_condition_index_ = 0;  // 0 = eliminate_all, 1 = reach_objective
    int new_map_width_ = 8;
    int new_map_height_ = 8;
};

}  // namespace chessadventure::editor
