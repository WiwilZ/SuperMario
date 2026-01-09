#pragma once

#include <chrono>
#include <cmath>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Enemy.hpp"
#include "Global.hpp"
#include "Goomba.hpp"
#include "Koopa.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"

//One person asked, "Why don't you use Tiled Map Editor?"
//My answer is, "Why should I work hard, when I don't have to work hard?"
inline void convert_sketch(uint8_t current_level, uint16_t& level_finish, std::vector<std::shared_ptr<Enemy>>& enemies,
                           sf::Color& background_color, MapManager& map_manager, Mario& mario) {
    map_manager.update_map_sketch(current_level);
    map_manager.set_map_size(map_manager.get_map_sketch_width());

    //We divide the height by 3 because the sketch stores the level as 3 layers: blocks, entities, and background tiles.
    uint16_t map_height = floor(map_manager.get_map_sketch_height() / 3.f);

    background_color = map_manager.get_map_sketch_pixel(0, map_manager.get_map_sketch_height() - 1);

    for (uint16_t a = 0; a < map_manager.get_map_sketch_width(); a++) {
        for (uint16_t b = 0; b < 2 * map_height; b++) {
            sf::Color pixel = map_manager.get_map_sketch_pixel(a, b);

            if (b < map_height) {
                if (sf::Color(182, 73, 0) == pixel) {
                    map_manager.set_map_cell(a, b, Cell::Brick);
                } else if (sf::Color(255, 255, 0) == pixel) {
                    map_manager.set_map_cell(a, b, Cell::Coin);
                } else if (sf::Color(0, 146, 0) == pixel || sf::Color(0, 182, 0) == pixel || sf::Color(0, 219, 0) ==
                    pixel) {
                    //Multiple colors, because we need to know which part of the pipe we need to draw.
                    map_manager.set_map_cell(a, b, Cell::Pipe);
                } else if (sf::Color(255, 73, 85) == pixel || sf::Color(255, 146, 85) == pixel) {
                    map_manager.set_map_cell(a, b, Cell::QuestionBlock);
                } else if (sf::Color(0, 0, 0) == pixel || sf::Color(146, 73, 0) == pixel) {
                    map_manager.set_map_cell(a, b, Cell::Wall);
                } else {
                    map_manager.set_map_cell(a, b, Cell::Empty);

                    if (sf::Color(0, 255, 255) == pixel) {
                        level_finish = a;
                    }
                }
            } else {
                if (sf::Color(255, 0, 0) == pixel) {
                    mario.set_position(CELL_SIZE * a, CELL_SIZE * (b - map_height));
                } else if (sf::Color(182, 73, 0) == pixel) {
                    enemies.push_back(std::make_shared<Goomba>(sf::Color(0, 0, 85) == background_color, CELL_SIZE * a,
                                                               CELL_SIZE * (b - map_height)));
                } else if (sf::Color(0, 219, 0) == pixel) {
                    enemies.push_back(std::make_shared<Koopa>(sf::Color(0, 0, 85) == background_color, CELL_SIZE * a,
                                                              CELL_SIZE * (b - map_height)));
                }
            }
        }
    }
}
