#include <doctest/doctest.h>

#include "pieces/Piece.h"
#include "upgrades/SkillTree.h"

using namespace chessadventure;

namespace {
std::string AssetPath(const std::string& relative) {
    return std::string(CHESSADVENTURE_ASSETS_DIR) + "/" + relative;
}
}  // namespace

TEST_CASE("pawn skill tree loads and enforces prerequisites") {
    SkillTree tree = SkillTree::LoadFromFile(AssetPath("data/upgrades/pawn.json"));

    CHECK(tree.piece_id() == "pawn");
    CHECK(tree.FindNode("shield_bash") != nullptr);
    CHECK(tree.FindNode("does_not_exist") == nullptr);

    CHECK(tree.CanUnlock("sturdy_stance", {}));      // no prerequisites
    CHECK_FALSE(tree.CanUnlock("shield_bash", {}));  // requires sturdy_stance first
    CHECK(tree.CanUnlock("shield_bash", {"sturdy_stance"}));
    CHECK_FALSE(tree.CanUnlock("sturdy_stance", {"sturdy_stance"}));  // already unlocked
}

TEST_CASE("applying an upgrade node mutates a Piece's stats and skills") {
    SkillTree tree = SkillTree::LoadFromFile(AssetPath("data/upgrades/pawn.json"));
    Piece piece("pawn", Team::kPlayer, {0, 0}, MovePattern::kPawnForward, 12, 3, 1, 1);
    piece.GainLevel(5);  // give it skill points to spend

    const int hp_before = piece.max_hp();
    piece.ApplyUpgrade(*tree.FindNode("sturdy_stance"));
    CHECK(piece.max_hp() == hp_before + 4);
    CHECK(piece.HasUnlocked("sturdy_stance"));

    piece.ApplyUpgrade(*tree.FindNode("shield_bash"));
    CHECK(piece.skills().size() == 1);
    CHECK(piece.skills()[0] == "shield_bash");
}
