#pragma once

#include <cmath>

#include <SFML/Graphics.hpp>

#include "Global.hpp"


inline void draw_map(unsigned view_x, const sf::Image& map_sketch, sf::RenderWindow& window,
                     const sf::Texture& map_texture, const Map& map) {
    //OPTIMIZATION!
    //We'll only draw the part of the map that is visible on the screen.
    uint16_t map_end = ceil((view_x + SCREEN_WIDTH) / static_cast<float>(CELL_SIZE));
    uint16_t map_height = floor(map_sketch.getSize().y / 3.f);
    uint16_t map_start = floor(view_x / static_cast<float>(CELL_SIZE));

    sf::Sprite cell_sprite(map_texture);

    for (uint16_t a = map_start; a < map_end; a++) {
        for (uint16_t b = 0; b < map_height; b++) {
            uint16_t sprite_x = 0;
            uint16_t sprite_y = 0;

            sf::Color pixel = map_sketch.getPixel(a, b + 2 * map_height);
            sf::Color pixel_left(0, 0, 0, 0);
            sf::Color pixel_right(0, 0, 0, 0);
            sf::Color pixel_up(0, 0, 0, 0);

            cell_sprite.setPosition(CELL_SIZE * a, CELL_SIZE * b);

            //Ignore the empty pixels.
            if (255 == pixel.a) {
                //Here we're getting pixels around the pixel we're currently checking.
                if (0 < a) {
                    pixel_left = map_sketch.getPixel(a - 1, b + 2 * map_height);
                }
                if (0 < b) {
                    pixel_up = map_sketch.getPixel(a, b + 2 * map_height - 1);
                }
                if (a < map_sketch.getSize().x - 1) {
                    pixel_right = map_sketch.getPixel(1 + a, b + 2 * map_height);
                }

                //Nothing complicated here.
                //Just a bunch of if's and else's that determine the position of the tile we're gonna draw.
                //Yeah, I know it's bad.
                if (sf::Color(255, 255, 255) == pixel) { //Clouds
                    sprite_x = 8;
                    if (sf::Color(255, 255, 255) == pixel_up) {
                        sprite_y = 1;
                    }
                    if (sf::Color(255, 255, 255) == pixel_left) {
                        if (sf::Color(255, 255, 255) != pixel_right) {
                            sprite_x = 9;
                        }
                    } else if (sf::Color(255, 255, 255) == pixel_right) {
                        sprite_x = 7;
                    }
                } else if (sf::Color(146, 219, 0) == pixel) { //Grass
                    sprite_x = 5;
                    if (sf::Color(146, 219, 0) == pixel_left) {
                        if (sf::Color(146, 219, 0) != pixel_right) {
                            sprite_x = 6;
                        }
                    } else if (sf::Color(146, 219, 0) == pixel_right) {
                        sprite_x = 4;
                    }
                } else if (sf::Color(0, 73, 0) == pixel) { //Hills outline
                    sprite_y = 1;
                    if (sf::Color(0, 109, 0) == pixel_left) {
                        if (sf::Color(0, 109, 0) != pixel_right) {
                            sprite_x = 2;
                        }
                    } else if (sf::Color(0, 109, 0) == pixel_right) {
                        sprite_x = 1;
                    }
                } else if (sf::Color(0, 109, 0) == pixel) { //Hills
                    sprite_x = 4;
                    sprite_y = 1;
                    if (sf::Color(0, 73, 0) == pixel_left) {
                        sprite_x = 3;
                    } else if (sf::Color(0, 73, 0) == pixel_right) {
                        sprite_x = 5;
                    }
                } else if (sf::Color(109, 255, 85) == pixel) { //Flagpole
                    sprite_x = 12;
                    if (sf::Color(109, 255, 85) == pixel_up) {
                        sprite_y = 1;
                    }
                }

                cell_sprite.setTextureRect(
                    sf::IntRect(CELL_SIZE * sprite_x, CELL_SIZE * sprite_y, CELL_SIZE, CELL_SIZE));
                window.draw(cell_sprite);
            }

            //Yes, we're drawing the blocks separately from the background tiles.
            if (Cell::Empty != map[a][b]) {
                if (Cell::Pipe == map[a][b]) { //Pipes
                    if (Cell::Pipe == map[a][b - 1]) {
                        sprite_y = 1;
                    } else {
                        sprite_y = 0;
                    }

                    if (Cell::Pipe == map[a - 1][b]) {
                        sprite_x = 11;
                    } else {
                        sprite_x = 10;
                    }
                } else if (Cell::QuestionBlock == map[a][b]) { //Question blocks
                    sprite_x = 1;
                    sprite_y = 0;
                } else if (Cell::Wall == map[a][b]) {
                    sprite_y = 0;
                    if (sf::Color(0, 0, 0) == map_sketch.getPixel(a, b)) { //Walls
                        sprite_x = 2;
                    } else { //Solid blocks
                        sprite_x = 3;
                    }
                }

                cell_sprite.setTextureRect(
                    sf::IntRect(CELL_SIZE * sprite_x, CELL_SIZE * sprite_y, CELL_SIZE, CELL_SIZE));
                window.draw(cell_sprite);
            }
        }
    }
}
