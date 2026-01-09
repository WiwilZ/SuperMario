#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "Global.hpp"
#include "MapCollision.hpp"


inline uint8_t map_collision(float x, float y, const std::vector<Cell>& check_cells, const Map& map) noexcept {
    float cell_x = x / CELL_SIZE;
    float cell_y = y / CELL_SIZE;

    uint8_t output = 0;

    for (uint8_t a = 0; a < 4; a++) {
        int x;
        int y;

        switch (a) {
        case 0: //Top left cell
            x = floor(cell_x);
            y = floor(cell_y);
            break;
        case 1: //Top right cell
            x = ceil(cell_x);
            y = floor(cell_y);
            break;
        case 2: //Bottom left cell
            x = floor(cell_x);
            y = ceil(cell_y);
            break;
        default: //Bottom right cell
            x = ceil(cell_x);
            y = ceil(cell_y);
        }

        if (0 <= x && x < map.size()) {
            if (0 <= y && y < map[0].size()) {
                //Checking if the cell's type is one of the cell types we need to check.
                if (check_cells.end() != std::ranges::find(check_cells, map[x][y])) {
                    //In binary this will be 0001, 0010, 0100 or 1000.
                    output += pow(2, a);
                }
            }
        } else if (check_cells.end() != std::ranges::find(check_cells, Cell::Wall)) {
            //We're gonna assume that the map borders are walls.
            output += pow(2, a);
        }
    }

    return output;
}
