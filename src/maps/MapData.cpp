#include "maps/MapData.h"

namespace chessadventure {

MapData MapData::MakeEmpty(int width, int height) {
    MapData data;
    data.width = width;
    data.height = height;
    data.tiles.assign(static_cast<size_t>(width) * height, TileType::kFloor);
    return data;
}

}  // namespace chessadventure
