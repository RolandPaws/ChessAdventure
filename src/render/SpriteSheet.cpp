#include "render/SpriteSheet.h"

#include <SDL3_image/SDL_image.h>

namespace chessadventure {

bool SpriteSheet::LoadFromFile(SDL_Renderer* renderer, const std::string& path) {
    SDL_Texture* raw = IMG_LoadTexture(renderer, path.c_str());
    if (!raw)
        return false;
    texture_.reset(raw);
    return true;
}

void SpriteSheet::DefineFrame(const std::string& name, SDL_Rect rect) {
    frames_[name] = rect;
}

const SDL_Rect* SpriteSheet::FindFrame(const std::string& name) const {
    auto it = frames_.find(name);
    return it != frames_.end() ? &it->second : nullptr;
}

}  // namespace chessadventure
