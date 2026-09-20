# CLAUDE.md

Guidance for an AI coding agent (or a human) working in this repo. See
[README.md](README.md) for setup/build instructions and [ROADMAP.md](ROADMAP.md)
for what's built vs. planned.

## What this is

A pixel-art RPG-chess game: SDL3 for rendering/input, data-driven piece stats and
upgrade trees (JSON), a JSON map format shared between the game and a visual map
editor (`tools/map_editor`, Dear ImGui). C++20, CMake.

## Build / test loop

```bash
cmake --preset debug
cmake --build --preset debug -j
ctest --test-dir build/debug --output-on-failure
```

Run this after any change to `src/` or `tests/` before considering a task done.
The game and editor executables can be smoke-tested headlessly:

```bash
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 2 ./build/debug/ChessAdventure
```

(Exit code 124 from `timeout` means it ran the full 2s without crashing -- that's
the pass condition, since there's no window system to quit it normally.)

Format before considering any C++ change done:

```bash
find src tools tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

CI (`.github/workflows/ci.yml`) enforces `clang-format --dry-run --Werror`, build,
and `ctest` on every push/PR.

## Architecture boundaries -- keep these intact

- **`src/game`, `src/pieces`, `src/upgrades`, `src/maps` have no SDL dependency.**
  This is deliberate: it's what makes them unit-testable without a display and
  reusable from the map editor. Don't add `#include <SDL3/...>` to these.
- **`src/core` and `src/render` are the only places SDL-specific code belongs**
  (window/renderer lifecycle, event loop, drawing).
- **`tools/map_editor` and the game both depend on `chessadventure_core` and share
  the same `MapData`/`MapLoader`/`PieceDatabase`.** A map built in the editor must
  load in the game with zero conversion -- if you change the map JSON schema,
  update `MapLoader` (both load and save) and the editor's UI together.
- Tests load the **real** files under `assets/`, not fixture copies (see
  `tests/CMakeLists.txt`'s `CHESSADVENTURE_ASSETS_DIR` define). If you add a new
  piece or upgrade tree, a broken JSON file there should already fail `ctest`.

## Extending the data-driven systems

- **New piece type**: add an entry to `assets/data/pieces.json` (id, name,
  description, `movePattern` -- see `pieces/MoveRules.h` for the valid pattern
  strings, base stats) and a matching `assets/data/upgrades/<id>.json` skill tree
  (can start with a single node). No C++ changes needed for a stats-only piece.
- **New upgrade node**: add it to the relevant `assets/data/upgrades/*.json`.
  `statDeltas` fields are optional and default to 0. `grantsSkill` is a free-form
  string identifier; nothing currently reads it at runtime beyond storing it on
  the `Piece` (see ROADMAP Phase 2 -- wiring skill *effects* into combat is not
  built yet, so don't assume a granted skill does anything in-game today).
- **New tile type**: requires a C++ change -- add the enum value in `game/Tile.h`,
  the string mapping in `game/Tile.cpp`, a palette color in `render/Palette.h`,
  and a case in both `Application::DrawBoard` and the map editor's `ColorForTile`.
- **New map**: author JSON directly or use `tools/map_editor`; either way it must
  validate against `MapLoader::LoadFromFile` (width/height match the tile array
  size, spawn `pieceId`s exist in `pieces.json`).

## Code style

- `.clang-format` (Google-based, 100 col, 4-space indent) is the source of truth --
  run it, don't hand-format.
- Comments explain *why*, not *what* (see the project's general style: no
  restating what a well-named function already says). Existing files are a good
  reference for the expected density.
- Prefer small, focused headers per class (current layout: one class per
  `.h`/`.cpp` pair) over grouping unrelated things into a single file.

## Known rough edges (not bugs -- see ROADMAP.md)

- No turn structure, AI, or combat resolution beyond "moving onto an enemy square
  captures it" -- `Board::MovePiece` doesn't even check the mover's move pattern;
  that validation lives in `Application`/the caller via `MoveRules::ComputeLegalMoves`.
- No real sprites -- everything renders as flat-colored rects from
  `render/Palette.h`. See `docs/ART_PIPELINE.md` before adding art.
- The map editor has no undo/redo and can't resize an existing map in place
  (only via "New Map").
