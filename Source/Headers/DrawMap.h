#pragma once

#include <SFML/Graphics.hpp>

#include "Global.h"


inline void draw_map(unsigned view_x, const sf::Image& map_sketch, sf::RenderWindow& window,
                     const sf::Texture& map_texture, const Map& map) {
    //OPTIMIZATION!
    //We'll only draw the part of the map that is visible on the screen.
    uint16_t map_end = (view_x + SCREEN_WIDTH + CELL_SIZE - 1) / CELL_SIZE;
    uint16_t map_height = map_sketch.getSize().y / 3;
    uint16_t map_start = view_x / CELL_SIZE;

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
            if (pixel.a == 255) {
                //Here we're getting pixels around the pixel we're currently checking.
                if (a > 0) {
                    pixel_left = map_sketch.getPixel(a - 1, b + 2 * map_height);
                }
                if (b > 0) {
                    pixel_up = map_sketch.getPixel(a, b + 2 * map_height - 1);
                }
                if (a < map_sketch.getSize().x - 1) {
                    pixel_right = map_sketch.getPixel(1 + a, b + 2 * map_height);
                }

                //Nothing complicated here.
                //Just a bunch of if's and else's that determine the position of the tile we're gonna draw.
                //Yeah, I know it's bad.
                if (pixel == sf::Color(255, 255, 255)) { //Clouds
                    sprite_x = 8;
                    if (pixel_up == sf::Color(255, 255, 255)) {
                        sprite_y = 1;
                    }
                    if (pixel_left == sf::Color(255, 255, 255)) {
                        if (pixel_right != sf::Color(255, 255, 255)) {
                            sprite_x = 9;
                        }
                    } else if (pixel_right == sf::Color(255, 255, 255)) {
                        sprite_x = 7;
                    }
                } else if (pixel == sf::Color(146, 219, 0)) { //Grass
                    sprite_x = 5;
                    if (pixel_left == sf::Color(146, 219, 0)) {
                        if (pixel_right != sf::Color(146, 219, 0)) {
                            sprite_x = 6;
                        }
                    } else if (pixel_right == sf::Color(146, 219, 0)) {
                        sprite_x = 4;
                    }
                } else if (pixel == sf::Color(0, 73, 0)) { //Hills outline
                    sprite_y = 1;
                    if (pixel_left == sf::Color(0, 109, 0)) {
                        if (pixel_right != sf::Color(0, 109, 0)) {
                            sprite_x = 2;
                        }
                    } else if (pixel_right == sf::Color(0, 109, 0)) {
                        sprite_x = 1;
                    }
                } else if (pixel == sf::Color(0, 109, 0)) { //Hills
                    sprite_x = 4;
                    sprite_y = 1;
                    if (pixel_left == sf::Color(0, 73, 0)) {
                        sprite_x = 3;
                    } else if (pixel_right == sf::Color(0, 73, 0)) {
                        sprite_x = 5;
                    }
                } else if (pixel == sf::Color(109, 255, 85)) { //Flagpole
                    sprite_x = 12;
                    if (pixel_up == sf::Color(109, 255, 85)) {
                        sprite_y = 1;
                    }
                }

                cell_sprite.setTextureRect(sf::IntRect(
                    CELL_SIZE * sprite_x, CELL_SIZE * sprite_y, CELL_SIZE, CELL_SIZE
                ));
                window.draw(cell_sprite);
            }

            //Yes, we're drawing the blocks separately from the background tiles.
            if (map[a][b] != Cell::Empty) {
                if (map[a][b] == Cell::Pipe) { //Pipes
                    if (map[a][b - 1] == Cell::Pipe) {
                        sprite_y = 1;
                    } else {
                        sprite_y = 0;
                    }

                    if (map[a - 1][b] == Cell::Pipe) {
                        sprite_x = 11;
                    } else {
                        sprite_x = 10;
                    }
                } else if (map[a][b] == Cell::QuestionBlock) { //Question blocks
                    sprite_x = 1;
                    sprite_y = 0;
                } else if (map[a][b] == Cell::Wall) {
                    sprite_y = 0;
                    if (map_sketch.getPixel(a, b) == sf::Color(0, 0, 0)) { //Walls
                        sprite_x = 2;
                    } else { //Solid blocks
                        sprite_x = 3;
                    }
                }

                cell_sprite.setTextureRect(sf::IntRect(
                    CELL_SIZE * sprite_x, CELL_SIZE * sprite_y, CELL_SIZE, CELL_SIZE
                ));
                window.draw(cell_sprite);
            }
        }
    }
}
