#pragma once

#include <SDL3/SDL.h>

namespace chessadventure::palette {

// Placeholder retro/pixel-art palette used until real sprite sheets land in
// assets/sprites/ (see SpriteSheet.h and docs/ART_PIPELINE.md). Keeping this in one
// place means swapping in real art later only touches the renderer, not gameplay code.
inline constexpr SDL_Color kBackground{20, 18, 24, 255};
inline constexpr SDL_Color kBoardLight{224, 205, 169, 255};
inline constexpr SDL_Color kBoardDark{110, 79, 66, 255};
inline constexpr SDL_Color kBoardHighlight{247, 208, 84, 180};

inline constexpr SDL_Color kWall{58, 53, 63, 255};
inline constexpr SDL_Color kHazard{178, 63, 63, 255};
inline constexpr SDL_Color kPlayerSpawn{88, 140, 178, 120};
inline constexpr SDL_Color kEnemySpawn{178, 88, 88, 120};
inline constexpr SDL_Color kObjective{219, 180, 84, 140};

inline constexpr SDL_Color kPlayerPiece{92, 168, 214, 255};
inline constexpr SDL_Color kEnemyPiece{206, 84, 84, 255};
inline constexpr SDL_Color kPieceOutline{15, 14, 18, 255};

inline constexpr SDL_Color kHpBarBack{40, 36, 44, 255};
inline constexpr SDL_Color kHpBarFill{99, 178, 107, 255};

}  // namespace chessadventure::palette
