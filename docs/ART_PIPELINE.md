# Art pipeline (placeholder, until real sprites exist)

There is no shipped pixel art yet. Everything currently on screen (`src/render/Palette.h`)
is flat-colored rectangles standing in for sprites, so gameplay and UI code can be
written against real interfaces before art exists rather than being blocked on it.

## Current placeholder rendering

- `render/Palette.h` -- every color the renderer uses, one place to swap for a real
  palette later.
- `Application::DrawBoard/DrawPieces/DrawHighlights` (game) and
  `EditorApp::DrawCanvas` (map editor) both draw filled rects from that palette.

## When real art lands

`render/SpriteSheet.h` already has the loading interface (`LoadFromFile`,
`DefineFrame`, `FindFrame`) for a texture atlas with named sub-rects, using
SDL3_image. Suggested layout for the first pass:

- One PNG per piece type (`assets/sprites/<piece_id>.png`), each a horizontal strip
  of equal-sized frames: idle, move, attack, hurt, defeated.
- A parallel `assets/sprites/<piece_id>.json` (or an entry added to `pieces.json`)
  declaring frame size and named frame indices, loaded into `SpriteSheet` via
  repeated `DefineFrame` calls.
- Tile art as a single small atlas (`assets/sprites/tiles.png`) since there are far
  fewer tile types than piece animations.
- Keep sprites pixel-native resolution (e.g. 32x32 or 48x48) and scale up with
  nearest-neighbor sampling (`SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)`)
  so scaling stays crisp -- never author or export pre-smoothed/anti-aliased frames.

Swapping in real art should only touch the `Draw*` methods (reading from a loaded
`SpriteSheet` instead of `SDL_RenderFillRect`) -- it should not require changes to
`Board`, `Piece`, `MoveRules`, or the map format.
