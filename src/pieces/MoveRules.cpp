#include "pieces/MoveRules.h"

#include <array>
#include <unordered_map>

namespace chessadventure {

MovePattern MovePatternFromString(const std::string& name) {
    static const std::unordered_map<std::string, MovePattern> kLookup = {
        {"orthogonal_any", MovePattern::kOrthogonalAny},
        {"diagonal_any", MovePattern::kDiagonalAny},
        {"omni_any", MovePattern::kOmniAny},
        {"omni_one", MovePattern::kOmniOne},
        {"knight_jump", MovePattern::kKnightJump},
        {"pawn_forward", MovePattern::kPawnForward},
    };
    auto it = kLookup.find(name);
    return it != kLookup.end() ? it->second : MovePattern::kOmniOne;
}

bool IsInBounds(Position pos, int board_width, int board_height) {
    return pos.x >= 0 && pos.x < board_width && pos.y >= 0 && pos.y < board_height;
}

namespace {

// Walks outward from `from` along each direction in `directions`, stopping at the
// board edge, at the first blocking piece (included only if capturable), or after
// `max_steps` squares.
void WalkDirections(const MoveContext& ctx,
                    const std::vector<Position>& directions,
                    int max_steps,
                    std::vector<Position>* out) {
    for (const Position& dir : directions) {
        Position cur = ctx.from;
        for (int step = 0; step < max_steps; ++step) {
            cur.x += dir.x;
            cur.y += dir.y;
            if (!IsInBounds(cur, ctx.board_width, ctx.board_height))
                break;
            if (ctx.is_occupied && ctx.is_occupied(cur)) {
                if (ctx.is_enemy && ctx.is_enemy(cur))
                    out->push_back(cur);
                break;
            }
            out->push_back(cur);
        }
    }
}

const std::vector<Position>& OrthogonalDirections() {
    static const std::vector<Position> kDirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    return kDirs;
}

const std::vector<Position>& DiagonalDirections() {
    static const std::vector<Position> kDirs = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    return kDirs;
}

}  // namespace

std::vector<Position> ComputeLegalMoves(MovePattern pattern, const MoveContext& ctx) {
    std::vector<Position> moves;

    switch (pattern) {
        case MovePattern::kOrthogonalAny:
            WalkDirections(ctx, OrthogonalDirections(), ctx.move_range, &moves);
            break;

        case MovePattern::kDiagonalAny:
            WalkDirections(ctx, DiagonalDirections(), ctx.move_range, &moves);
            break;

        case MovePattern::kOmniAny: {
            WalkDirections(ctx, OrthogonalDirections(), ctx.move_range, &moves);
            WalkDirections(ctx, DiagonalDirections(), ctx.move_range, &moves);
            break;
        }

        case MovePattern::kOmniOne: {
            std::vector<Position> all_dirs = OrthogonalDirections();
            const auto& diag = DiagonalDirections();
            all_dirs.insert(all_dirs.end(), diag.begin(), diag.end());
            WalkDirections(ctx, all_dirs, 1, &moves);
            break;
        }

        case MovePattern::kKnightJump: {
            static const std::array<Position, 8> kOffsets = {
                Position{1, 2},
                Position{2, 1},
                Position{2, -1},
                Position{1, -2},
                Position{-1, -2},
                Position{-2, -1},
                Position{-2, 1},
                Position{-1, 2},
            };
            for (const Position& offset : kOffsets) {
                Position dest{ctx.from.x + offset.x, ctx.from.y + offset.y};
                if (!IsInBounds(dest, ctx.board_width, ctx.board_height))
                    continue;
                if (ctx.is_occupied && ctx.is_occupied(dest)) {
                    if (ctx.is_enemy && ctx.is_enemy(dest))
                        moves.push_back(dest);
                    continue;
                }
                moves.push_back(dest);
            }
            break;
        }

        case MovePattern::kPawnForward: {
            const int dir = ctx.pawn_forward_dir >= 0 ? 1 : -1;
            Position step{ctx.from.x, ctx.from.y + dir};
            if (IsInBounds(step, ctx.board_width, ctx.board_height) &&
                !(ctx.is_occupied && ctx.is_occupied(step))) {
                moves.push_back(step);
            }
            for (int dx : {-1, 1}) {
                Position diag{ctx.from.x + dx, ctx.from.y + dir};
                if (!IsInBounds(diag, ctx.board_width, ctx.board_height))
                    continue;
                if (ctx.is_occupied && ctx.is_occupied(diag) && ctx.is_enemy &&
                    ctx.is_enemy(diag)) {
                    moves.push_back(diag);
                }
            }
            break;
        }
    }

    return moves;
}

}  // namespace chessadventure
