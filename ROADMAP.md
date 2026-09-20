# Roadmap

Rough phases, roughly in order. Not a commitment or a schedule -- just the current
plan of attack so contributors (human or agent) know what "next" means.

## Phase 0 -- Skeleton (done)

- [x] CMake project, SDL3 window + render loop
- [x] Board / Tile / Position primitives
- [x] Data-driven piece templates (`assets/data/pieces.json`)
- [x] Classic move patterns (`pieces/MoveRules.h`) with click-to-move in-game
- [x] Data-driven upgrade trees per piece (`assets/data/upgrades/*.json`)
- [x] JSON map format + loader (`maps/MapData.h`, `maps/MapLoader.h`)
- [x] Minimal visual map editor (`tools/map_editor`) that reads/writes that format
- [x] Unit tests for move rules, board, skill trees, map round-tripping

## Phase 1 -- Make it a game

- [ ] Turn structure: alternate player/enemy turns, end-turn button
- [ ] Basic enemy AI (even a greedy heuristic beats no opponent)
- [ ] Combat resolution beyond "capture on move" (attack vs. move as separate actions)
- [ ] Win/lose detection from `MapStory.victory_condition`, "you win/lose" screen
- [ ] Leveling: award skill points on kill/victory, in-game UI to spend them on a
      piece's skill tree (SkillTree/Piece already support this; no UI yet)

## Phase 2 -- Make it an RPG

- [ ] Skill *effects*, not just stat deltas -- `grantsSkill` strings currently do
      nothing at runtime; wire them into combat (shield_bash, minor_heal, etc.)
- [ ] Persistent roster across maps (a piece's level/upgrades survive between battles)
- [ ] Simple dialogue/story presentation using `MapStory.intro` / `on_victory`
- [ ] A campaign = an ordered list of maps with shared roster state between them

## Phase 3 -- Make it pretty

- [ ] Replace placeholder palette rendering (`render/Palette.h`) with real pixel art
      via `render/SpriteSheet.h` -- see `docs/ART_PIPELINE.md` for the intended
      sprite sheet layout once art exists
- [ ] Animation (idle/move/attack frames), simple particle/flash feedback on hits
- [ ] Audio (SDL3_mixer or similar) for moves, captures, victory/defeat stings

## Phase 4 -- Make it moddable

- [ ] Map editor: undo/redo, copy-paste regions, multi-tile brush
- [ ] Map editor: playtest button (spins up a Board from the in-editor MapData
      without leaving the tool, using the same `BuildBoardFromMap` the game uses)
- [ ] Custom piece templates authorable without editing raw JSON by hand
- [ ] Map/campaign packaging so a finished custom story can be shared as one file
