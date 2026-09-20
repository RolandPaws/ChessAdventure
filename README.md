# Chess Adventure

A pixel-art, RPG-flavored take on chess: many piece types, each with its own stat
line, upgrade tree, and unlockable skills, fighting across custom maps with their
own story beats. This repo is the **skeleton** -- a playable board with click-to-move,
a real data-driven upgrade system, a JSON map format, and a visual map editor to
build new maps without touching JSON by hand. See [ROADMAP.md](ROADMAP.md) for
what's built vs. what's next.

## Prerequisites

- A C++20 compiler (GCC 12+/Clang 15+/MSVC 19.3+)
- CMake 3.21+
- SDL3, SDL3_image, and SDL3_ttf development packages -- optional, see below

```bash
# Debian/Ubuntu (24.10+ / with SDL3 packaged)
sudo apt-get install build-essential cmake ninja-build pkg-config \
    libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev

# macOS (Homebrew)
brew install cmake sdl3 sdl3_image sdl3_ttf

# Windows
# Use vcpkg: vcpkg install sdl3 sdl3-image sdl3-ttf, then pass
# -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake to configure.
```

If SDL3/SDL3_image/SDL3_ttf aren't found on the system (e.g. on Ubuntu 24.04,
whose apt repos predate `libsdl3-dev`), CMake automatically fetches and builds
them from source instead -- no separate flag needed, see the SDL3 fetch block
in `CMakeLists.txt`. That path only needs `libfreetype-dev` on the system
(required by SDL3_ttf); `libharfbuzz-dev` is picked up automatically if present
but not required. This is what `ubuntu-latest` CI runners use.

Everything else (nlohmann/json, doctest, Dear ImGui) is always fetched
automatically by CMake at configure time -- no other manual setup.

## Build & run

```bash
cmake --preset debug
cmake --build --preset debug -j
./build/debug/ChessAdventure                       # the game
./build/debug/tools/map_editor/chessadventure_map_editor   # the map editor
```

Use `--preset release` instead of `debug` for an optimized build. Both presets
copy `assets/` next to the built executables automatically.

## Testing

```bash
ctest --test-dir build/debug --output-on-failure
```

Tests cover move-rule legality, board mechanics, skill-tree prerequisite logic,
and map JSON round-tripping -- and they load the real files in `assets/`, so a
broken data file fails the suite instead of only showing up at runtime.

## Project layout

```
src/
  core/       Application shell: SDL lifecycle, main loop, input, rendering
  game/       Board, Tile, Position -- the board state itself
  pieces/     Piece instances, move-pattern rules, the piece template database
  upgrades/   Skill tree data model (loaded from assets/data/upgrades/*.json)
  maps/       Map data model + JSON load/save, shared with the map editor
  render/     Palette (placeholder pixel-art colors) and SpriteSheet (for when
              real art lands -- see docs/ART_PIPELINE.md)
tools/
  map_editor/ Dear ImGui + SDL3 tool for building maps visually
tests/        doctest unit tests for the engine-agnostic src/ code
assets/
  data/       pieces.json (base stats/move patterns) + upgrades/*.json (skill trees)
  maps/       Map files in the shared JSON format
```

`chessadventure_core` (Board/Piece/MoveRules/SkillTree/MapLoader) has no SDL
dependency on purpose, so gameplay logic is fast to unit test and easy to reuse
from the map editor's preview tooling later.

## How pieces, upgrades, and maps fit together

- **Pieces** (`assets/data/pieces.json`) are base templates: a move pattern (rook-
  like, knight-like, pawn-like, ...) and base HP/attack/defense/move-range.
- **Upgrade trees** (`assets/data/upgrades/<piece_id>.json`) are a DAG of nodes per
  piece type. Each node costs skill points, may require other nodes first, and
  grants stat deltas and/or a named skill string. `Piece::ApplyUpgrade` applies a
  node; `SkillTree::CanUnlock` checks prerequisites.
- **Maps** (`assets/maps/*.json`) declare a tile grid, a list of piece spawns
  (which template, which team, where, starting level), and story metadata (intro
  text, victory condition, victory text). `BuildBoardFromMap` turns a `MapData` +
  `PieceDatabase` into a live, playable `Board`.
- The **map editor** reads and writes that exact same JSON format, so a map built
  visually loads directly into the game with no conversion step.

## Contributing / agentic development

See [CLAUDE.md](CLAUDE.md) for conventions this repo expects an AI coding agent
(or a human) to follow: build/test commands, code style, and architectural
boundaries to respect when extending the skeleton.

## License

[MIT](LICENSE)
