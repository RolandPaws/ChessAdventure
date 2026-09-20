#include "core/Application.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>

#include "core/Time.h"
#include "maps/MapLoader.h"
#include "pieces/MoveRules.h"
#include "render/Palette.h"

namespace chessadventure {

Application::Application(int window_width, int window_height)
    : window_width_(window_width), window_height_(window_height) {}

Application::~Application() {
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool Application::Init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow("Chess Adventure", window_width_, window_height_, 0);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_SetRenderLogicalPresentation(
        renderer_, window_width_, window_height_, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    // Pixel art stays crisp when scaled: nearest-neighbor sampling is set per-texture
    // via SDL_SetTextureScaleMode(..., SDL_SCALEMODE_NEAREST) once SpriteSheet loads
    // real sprites (no global hint for this in SDL3).

    return true;
}

bool Application::LoadContent(const std::string& piece_db_path, const std::string& map_path) {
    try {
        piece_db_ = std::make_unique<PieceDatabase>(PieceDatabase::LoadFromFile(piece_db_path));
        MapData map = MapLoader::LoadFromFile(map_path);
        board_ = std::make_unique<Board>(BuildBoardFromMap(map, *piece_db_));
        tile_size_ = window_width_ / std::max(board_->width(), 1);
    } catch (const std::exception& e) {
        std::cerr << "LoadContent failed: " << e.what() << "\n";
        return false;
    }
    return true;
}

int Application::Run() {
    running_ = true;
    Clock clock;

    while (running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            HandleEvent(event);
        }

        const float delta_seconds = clock.Tick();
        Update(delta_seconds);
        Render();
    }

    return 0;
}

void Application::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            running_ = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE)
                running_ = false;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (!event.button.down || !board_)
                break;
            const Position clicked = ScreenToBoard(event.button.x, event.button.y);
            if (!board_->IsInside(clicked))
                break;

            if (selected_ && !legal_moves_for_selected_.empty()) {
                bool moved = false;
                for (const Position& move : legal_moves_for_selected_) {
                    if (move == clicked) {
                        board_->MovePiece(*selected_, clicked);
                        moved = true;
                        break;
                    }
                }
                selected_.reset();
                legal_moves_for_selected_.clear();
                if (moved)
                    break;
            }

            if (Piece* piece = board_->PieceAt(clicked)) {
                selected_ = clicked;
                MoveContext ctx;
                ctx.from = clicked;
                ctx.board_width = board_->width();
                ctx.board_height = board_->height();
                ctx.move_range = piece->move_range();
                ctx.pawn_forward_dir = piece->team() == Team::kPlayer ? -1 : 1;
                ctx.is_occupied = [this](Position p) { return board_->PieceAt(p) != nullptr; };
                ctx.is_enemy = [this, piece](Position p) {
                    Piece* other = board_->PieceAt(p);
                    return other && other->team() != piece->team();
                };
                legal_moves_for_selected_ = ComputeLegalMoves(piece->move_pattern(), ctx);
            }
            break;
        }

        default:
            break;
    }
}

void Application::Update(float /*delta_seconds*/) {
    // Reserved for animation/AI/story-trigger ticking as those systems are built.
}

Position Application::ScreenToBoard(float screen_x, float screen_y) const {
    return Position{static_cast<int>(screen_x) / tile_size_,
                    static_cast<int>(screen_y) / tile_size_};
}

void Application::Render() {
    SDL_SetRenderDrawColor(renderer_,
                           palette::kBackground.r,
                           palette::kBackground.g,
                           palette::kBackground.b,
                           palette::kBackground.a);
    SDL_RenderClear(renderer_);

    if (board_) {
        DrawBoard();
        DrawHighlights();
        DrawPieces();
    }

    SDL_RenderPresent(renderer_);
}

void Application::DrawBoard() {
    for (int y = 0; y < board_->height(); ++y) {
        for (int x = 0; x < board_->width(); ++x) {
            const SDL_Color base = ((x + y) % 2 == 0) ? palette::kBoardLight : palette::kBoardDark;
            SDL_Color color = base;
            switch (board_->TileAt({x, y}).type) {
                case TileType::kWall:
                    color = palette::kWall;
                    break;
                case TileType::kHazard:
                    color = palette::kHazard;
                    break;
                case TileType::kPlayerSpawn:
                    color = palette::kPlayerSpawn;
                    break;
                case TileType::kEnemySpawn:
                    color = palette::kEnemySpawn;
                    break;
                case TileType::kObjective:
                    color = palette::kObjective;
                    break;
                case TileType::kFloor:
                    break;
            }
            SDL_FRect rect{static_cast<float>(x * tile_size_),
                           static_cast<float>(y * tile_size_),
                           static_cast<float>(tile_size_),
                           static_cast<float>(tile_size_)};
            SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer_, &rect);
        }
    }
}

void Application::DrawHighlights() {
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    for (const Position& pos : legal_moves_for_selected_) {
        SDL_FRect rect{static_cast<float>(pos.x * tile_size_),
                       static_cast<float>(pos.y * tile_size_),
                       static_cast<float>(tile_size_),
                       static_cast<float>(tile_size_)};
        SDL_SetRenderDrawColor(renderer_,
                               palette::kBoardHighlight.r,
                               palette::kBoardHighlight.g,
                               palette::kBoardHighlight.b,
                               palette::kBoardHighlight.a);
        SDL_RenderFillRect(renderer_, &rect);
    }
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void Application::DrawPieces() {
    const float margin = tile_size_ * 0.15f;
    for (const auto& piece : board_->pieces()) {
        const Position pos = piece->position();
        const SDL_Color color =
            piece->team() == Team::kPlayer ? palette::kPlayerPiece : palette::kEnemyPiece;

        SDL_FRect body{static_cast<float>(pos.x * tile_size_) + margin,
                       static_cast<float>(pos.y * tile_size_) + margin,
                       static_cast<float>(tile_size_) - 2 * margin,
                       static_cast<float>(tile_size_) - 2 * margin};
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer_, &body);

        // HP bar.
        const float hp_ratio =
            piece->max_hp() > 0 ? static_cast<float>(piece->current_hp()) / piece->max_hp() : 0.0f;
        SDL_FRect bar_back{body.x, body.y - 6, body.w, 4};
        SDL_SetRenderDrawColor(renderer_,
                               palette::kHpBarBack.r,
                               palette::kHpBarBack.g,
                               palette::kHpBarBack.b,
                               palette::kHpBarBack.a);
        SDL_RenderFillRect(renderer_, &bar_back);

        SDL_FRect bar_fill{body.x, body.y - 6, body.w * hp_ratio, 4};
        SDL_SetRenderDrawColor(renderer_,
                               palette::kHpBarFill.r,
                               palette::kHpBarFill.g,
                               palette::kHpBarFill.b,
                               palette::kHpBarFill.a);
        SDL_RenderFillRect(renderer_, &bar_fill);
    }
}

}  // namespace chessadventure
