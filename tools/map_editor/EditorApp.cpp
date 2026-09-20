#include "EditorApp.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cstring>
#include <iostream>

#include "maps/MapLoader.h"
#include "render/Palette.h"

namespace chessadventure::editor {

namespace {

constexpr float kSidePanelWidth = 340.0f;

void CopyToBuffer(const std::string& src, char* dst, size_t dst_size) {
    std::strncpy(dst, src.c_str(), dst_size - 1);
    dst[dst_size - 1] = '\0';
}

SDL_Color ColorForTile(TileType type) {
    switch (type) {
        case TileType::kWall:
            return palette::kWall;
        case TileType::kHazard:
            return palette::kHazard;
        case TileType::kPlayerSpawn:
            return palette::kPlayerSpawn;
        case TileType::kEnemySpawn:
            return palette::kEnemySpawn;
        case TileType::kObjective:
            return palette::kObjective;
        case TileType::kFloor:
            return palette::kBoardLight;
    }
    return palette::kBoardLight;
}

}  // namespace

EditorApp::EditorApp() = default;

EditorApp::~EditorApp() {
    if (renderer_) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer_);
    }
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool EditorApp::Init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow("Chess Adventure -- Map Editor", 1280, 800, SDL_WINDOW_RESIZABLE);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);

    try {
        piece_db_ = PieceDatabase::LoadFromFile("assets/data/pieces.json");
        if (!piece_db_.templates().empty()) {
            selected_piece_id_ = piece_db_.templates().begin()->first;
        }
        status_message_ = "Loaded piece database.";
    } catch (const std::exception& e) {
        status_message_ = std::string("Failed to load piece database: ") + e.what();
        std::cerr << status_message_ << "\n";
    }

    CopyToBuffer(map_.name, name_buffer_.data(), name_buffer_.size());
    return true;
}

int EditorApp::Run() {
    running_ = true;
    while (running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            HandleEvent(event);
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        DrawUi();

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer_,
                               palette::kBackground.r,
                               palette::kBackground.g,
                               palette::kBackground.b,
                               palette::kBackground.a);
        SDL_RenderClear(renderer_);

        DrawCanvas();

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
        SDL_RenderPresent(renderer_);
    }
    return 0;
}

void EditorApp::HandleEvent(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.down && !io.WantCaptureMouse) {
        const Position tile_pos = CanvasPixelToTile(event.button.x, event.button.y);
        if (tile_pos.x < 0 || tile_pos.x >= map_.width || tile_pos.y < 0 ||
            tile_pos.y >= map_.height) {
            return;
        }

        switch (paint_mode_) {
            case PaintMode::kTile:
                map_.SetTile(tile_pos.x, tile_pos.y, selected_tile_type_);
                break;

            case PaintMode::kErase: {
                auto& spawns = map_.spawns;
                spawns.erase(std::remove_if(spawns.begin(),
                                            spawns.end(),
                                            [&](const PieceSpawn& s) {
                                                return s.x == tile_pos.x && s.y == tile_pos.y;
                                            }),
                             spawns.end());
                map_.SetTile(tile_pos.x, tile_pos.y, TileType::kFloor);
                break;
            }

            case PaintMode::kPlacePiece: {
                if (selected_piece_id_.empty())
                    break;
                auto& spawns = map_.spawns;
                spawns.erase(std::remove_if(spawns.begin(),
                                            spawns.end(),
                                            [&](const PieceSpawn& s) {
                                                return s.x == tile_pos.x && s.y == tile_pos.y;
                                            }),
                             spawns.end());
                PieceSpawn spawn;
                spawn.piece_id = selected_piece_id_;
                spawn.team = placing_for_enemy_team_ ? "enemy" : "player";
                spawn.x = tile_pos.x;
                spawn.y = tile_pos.y;
                spawn.level = 1;
                spawns.push_back(spawn);
                break;
            }
        }
    }
}

SDL_FRect EditorApp::CanvasRect() const {
    int win_w = 0, win_h = 0;
    SDL_GetWindowSize(window_, &win_w, &win_h);
    return SDL_FRect{kSidePanelWidth,
                     0.0f,
                     static_cast<float>(win_w) - kSidePanelWidth,
                     static_cast<float>(win_h)};
}

Position EditorApp::CanvasPixelToTile(float x, float y) const {
    const SDL_FRect canvas = CanvasRect();
    return Position{static_cast<int>((x - canvas.x) / tile_size_),
                    static_cast<int>((y - canvas.y) / tile_size_)};
}

void EditorApp::DrawCanvas() {
    const SDL_FRect canvas = CanvasRect();
    tile_size_ = std::max(
        8.0f, std::min(canvas.w / std::max(map_.width, 1), canvas.h / std::max(map_.height, 1)));

    for (int y = 0; y < map_.height; ++y) {
        for (int x = 0; x < map_.width; ++x) {
            SDL_Color color = ColorForTile(map_.TileAt(x, y));
            if (map_.TileAt(x, y) == TileType::kFloor && (x + y) % 2 == 1) {
                color = palette::kBoardDark;
            }
            SDL_FRect rect{
                canvas.x + x * tile_size_, canvas.y + y * tile_size_, tile_size_, tile_size_};
            SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer_, &rect);
        }
    }

    const float margin = tile_size_ * 0.15f;
    for (const auto& spawn : map_.spawns) {
        const SDL_Color color =
            spawn.team == "enemy" ? palette::kEnemyPiece : palette::kPlayerPiece;
        SDL_FRect body{canvas.x + spawn.x * tile_size_ + margin,
                       canvas.y + spawn.y * tile_size_ + margin,
                       tile_size_ - 2 * margin,
                       tile_size_ - 2 * margin};
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer_, &body);
    }
}

void EditorApp::DrawUi() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    int win_w = 0, win_h = 0;
    SDL_GetWindowSize(window_, &win_w, &win_h);
    ImGui::SetNextWindowSize(ImVec2(kSidePanelWidth, static_cast<float>(win_h)));

    ImGui::Begin("Map Editor",
                 nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::CollapsingHeader("Map", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Name", name_buffer_.data(), name_buffer_.size());
        map_.name = name_buffer_.data();

        ImGui::InputInt("Width", &new_map_width_);
        ImGui::InputInt("Height", &new_map_height_);
        new_map_width_ = std::clamp(new_map_width_, 2, 32);
        new_map_height_ = std::clamp(new_map_height_, 2, 32);
        if (ImGui::Button("New Map")) {
            NewMap(new_map_width_, new_map_height_);
        }
    }

    if (ImGui::CollapsingHeader("File", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Path", path_buffer_.data(), path_buffer_.size());
        if (ImGui::Button("Load"))
            LoadMap(path_buffer_.data());
        ImGui::SameLine();
        if (ImGui::Button("Save"))
            SaveMap(path_buffer_.data());
    }

    if (ImGui::CollapsingHeader("Paint Tiles", ImGuiTreeNodeFlags_DefaultOpen)) {
        static const std::pair<const char*, TileType> kTileOptions[] = {
            {"Floor", TileType::kFloor},
            {"Wall", TileType::kWall},
            {"Hazard", TileType::kHazard},
            {"Player Spawn", TileType::kPlayerSpawn},
            {"Enemy Spawn", TileType::kEnemySpawn},
            {"Objective", TileType::kObjective},
        };
        for (const auto& [label, type] : kTileOptions) {
            const bool selected = paint_mode_ == PaintMode::kTile && selected_tile_type_ == type;
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.55f, 0.3f, 1.0f));
            if (ImGui::Button(label)) {
                paint_mode_ = PaintMode::kTile;
                selected_tile_type_ = type;
            }
            if (selected)
                ImGui::PopStyleColor();
        }
    }

    if (ImGui::CollapsingHeader("Place Pieces", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enemy team", &placing_for_enemy_team_);
        for (const auto& [id, tmpl] : piece_db_.templates()) {
            const bool selected = paint_mode_ == PaintMode::kPlacePiece && selected_piece_id_ == id;
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.55f, 0.3f, 1.0f));
            if (ImGui::Button(tmpl.name.c_str())) {
                paint_mode_ = PaintMode::kPlacePiece;
                selected_piece_id_ = id;
            }
            if (selected)
                ImGui::PopStyleColor();
        }
        if (ImGui::Button("Erase Tool")) {
            paint_mode_ = PaintMode::kErase;
        }
    }

    if (ImGui::CollapsingHeader("Story")) {
        ImGui::InputTextMultiline(
            "Intro", intro_buffer_.data(), intro_buffer_.size(), ImVec2(-1, 60));
        map_.story.intro = intro_buffer_.data();

        const char* kVictoryOptions[] = {"eliminate_all", "reach_objective"};
        ImGui::Combo("Victory Condition",
                     &victory_condition_index_,
                     kVictoryOptions,
                     IM_ARRAYSIZE(kVictoryOptions));
        map_.story.victory_condition = kVictoryOptions[victory_condition_index_];

        ImGui::InputTextMultiline(
            "On Victory", on_victory_buffer_.data(), on_victory_buffer_.size(), ImVec2(-1, 60));
        map_.story.on_victory = on_victory_buffer_.data();
    }

    ImGui::Separator();
    ImGui::TextWrapped("%s", status_message_.c_str());

    ImGui::End();
}

void EditorApp::NewMap(int width, int height) {
    map_ = MapData::MakeEmpty(width, height);
    CopyToBuffer(map_.name, name_buffer_.data(), name_buffer_.size());
    status_message_ =
        "Created a new " + std::to_string(width) + "x" + std::to_string(height) + " map.";
}

void EditorApp::LoadMap(const std::string& path) {
    try {
        map_ = MapLoader::LoadFromFile(path);
        CopyToBuffer(map_.name, name_buffer_.data(), name_buffer_.size());
        CopyToBuffer(map_.story.intro, intro_buffer_.data(), intro_buffer_.size());
        CopyToBuffer(map_.story.on_victory, on_victory_buffer_.data(), on_victory_buffer_.size());
        victory_condition_index_ = map_.story.victory_condition == "reach_objective" ? 1 : 0;
        status_message_ = "Loaded " + path;
    } catch (const std::exception& e) {
        status_message_ = std::string("Failed to load: ") + e.what();
    }
}

void EditorApp::SaveMap(const std::string& path) {
    try {
        MapLoader::SaveToFile(map_, path);
        status_message_ = "Saved " + path;
    } catch (const std::exception& e) {
        status_message_ = std::string("Failed to save: ") + e.what();
    }
}

}  // namespace chessadventure::editor
