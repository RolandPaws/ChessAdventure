#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace chessadventure {

// A rectangular region within a sprite sheet texture, named for lookup (e.g.
// "pawn_idle_0"). Frame rects are defined once an atlas layout is chosen; this
// skeleton has no shipped art yet, so nothing calls DefineFrame in production
// code — it exists so gameplay/render code can be written against the final
// interface before pixel art is ready. See docs/ART_PIPELINE.md.
class SpriteSheet {
public:
    // Loads a single PNG (or any SDL_image-supported format) as one texture.
    // Returns false (and leaves the sheet unloaded) if the file can't be read —
    // callers should fall back to palette-based placeholder rendering.
    bool LoadFromFile(SDL_Renderer* renderer, const std::string& path);

    void DefineFrame(const std::string& name, SDL_Rect rect);
    const SDL_Rect* FindFrame(const std::string& name) const;

    SDL_Texture* texture() const {
        return texture_.get();
    }
    bool IsLoaded() const {
        return texture_ != nullptr;
    }

private:
    struct TextureDeleter {
        void operator()(SDL_Texture* t) const {
            if (t)
                SDL_DestroyTexture(t);
        }
    };

    std::unique_ptr<SDL_Texture, TextureDeleter> texture_;
    std::unordered_map<std::string, SDL_Rect> frames_;
};

}  // namespace chessadventure
