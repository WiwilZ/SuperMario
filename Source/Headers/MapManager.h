#pragma once

#include <array>
#include <chrono>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Animation.h"
#include "Global.h"
#include "MapManager.h"
#include "Utils.h"


class MapManager {
    std::vector<Object> brick_particles;
    std::vector<Object> question_block_coins;

    sf::Image map_sketch;

    sf::Texture map_texture;
    sf::Sprite cell_sprite;

    Animation coin_animation;
    Animation question_block_animation;

    Map map;

public:
    MapManager() :
        coin_animation(CELL_SIZE, "Resources/Images/Coin.png", COIN_ANIMATION_SPEED),
        question_block_animation(CELL_SIZE, "Resources/Images/QuestionBlock.png", QUESTION_BLOCK_ANIMATION_SPEED) {
        map_texture.loadFromFile("Resources/Images/Map.png");
        cell_sprite.setTexture(map_texture);
    }

    uint16_t get_map_sketch_height() const {
        return map_sketch.getSize().y;
    }

    uint16_t get_map_sketch_width() const {
        return map_sketch.getSize().x;
    }

    uint16_t get_map_width() const {
        return map.size();
    }

    void add_brick_particles(uint16_t x, uint16_t y) {
        //Adding brick particles.
        //I was too lazy to add randomness.
        //It still looks cool, in my opinion.
        brick_particles.emplace_back(x, y, -0.25f * BRICK_PARTICLE_SPEED, -1.5f * BRICK_PARTICLE_SPEED);
        brick_particles.emplace_back(x + 0.5f * CELL_SIZE, y, 0.25f * BRICK_PARTICLE_SPEED,
                                     -1.5f * BRICK_PARTICLE_SPEED);
        brick_particles.emplace_back(x, y + 0.5f * CELL_SIZE, -0.5f * BRICK_PARTICLE_SPEED,
                                     -BRICK_PARTICLE_SPEED);
        brick_particles.emplace_back(x + 0.5f * CELL_SIZE, y + 0.5f * CELL_SIZE, 0.5f * BRICK_PARTICLE_SPEED,
                                     -BRICK_PARTICLE_SPEED);
    }

    void add_question_block_coin(uint16_t x, uint16_t y) {
        question_block_coins.emplace_back(x, y, 0, COIN_JUMP_SPEED);
    }

    void draw_map(bool draw_background, bool is_underground, unsigned view_x, sf::RenderWindow& window) {
        uint16_t map_end = (SCREEN_WIDTH + view_x + CELL_SIZE - 1) / CELL_SIZE;
        uint16_t map_height = map_sketch.getSize().y / 3;
        uint16_t map_start = view_x / CELL_SIZE;

        //We're drawing the coin before drawing the blocks because we want it to appear behind the question block.
        if (!draw_background) {
            for (const Object& question_block_coin : question_block_coins) {
                coin_animation.set_position(question_block_coin.x, question_block_coin.y);
                coin_animation.draw(window);
            }
        }

        for (uint16_t a = map_start; a < map_end; a++) {
            for (uint16_t b = 0; b < map_height; b++) {
                uint16_t sprite_x = 0;
                uint16_t sprite_y = 0;

                cell_sprite.setPosition(CELL_SIZE * a, CELL_SIZE * b);

                //This code is a big mess.
                //But it works.
                //Keep that in mind before judging me.
                if (draw_background) {
                    sf::Color pixel = map_sketch.getPixel(a, b + 2 * map_height);
                    sf::Color pixel_left(0, 0, 0, 0);
                    sf::Color pixel_right(0, 0, 0, 0);
                    sf::Color pixel_up(0, 0, 0, 0);

                    if (pixel.a == 255) {
                        if (a > 0) {
                            pixel_left = map_sketch.getPixel(a - 1, b + 2 * map_height);
                        }
                        if (b > 0) {
                            pixel_up = map_sketch.getPixel(a, b + 2 * map_height - 1);
                        }
                        if (a < map_sketch.getSize().x - 1) {
                            pixel_right = map_sketch.getPixel(1 + a, b + 2 * map_height);
                        }

                        if (pixel == sf::Color(255, 255, 255)) {
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
                        } else if (pixel == sf::Color(146, 219, 0)) {
                            sprite_x = 5;
                        } else if (pixel == sf::Color(146, 146, 0)) {
                            sprite_x = 4;
                        } else if (pixel == sf::Color(146, 182, 0)) {
                            sprite_x = 6;
                        } else if (pixel == sf::Color(0, 73, 0)) {
                            sprite_y = 1;
                            if (pixel_left == sf::Color(0, 109, 0)) {
                                if (pixel_right != sf::Color(0, 109, 0)) {
                                    sprite_x = 2;
                                }
                            } else if (pixel_right == sf::Color(0, 109, 0)) {
                                sprite_x = 1;
                            }
                        } else if (pixel == sf::Color(0, 109, 0)) {
                            sprite_y = 1;
                            if (pixel_left == sf::Color(0, 73, 0)) {
                                sprite_x = 3;
                            } else if (pixel_right == sf::Color(0, 73, 0)) {
                                sprite_x = 5;
                            } else {
                                sprite_x = 4;
                            }
                        } else if (pixel == sf::Color(109, 255, 85)) {
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
                } else if (map[a][b] != Cell::Empty) {
                    if (map[a][b] == Cell::Coin) {
                        coin_animation.set_position(CELL_SIZE * a, CELL_SIZE * b);
                        coin_animation.draw(window);
                    } else if (map[a][b] == Cell::QuestionBlock) {
                        question_block_animation.set_position(CELL_SIZE * a, CELL_SIZE * b);
                        question_block_animation.draw(window);
                    } else {
                        //Since the underground blocks have a different look, I placed their texture 2 cells below the regular ones in the map texture.
                        sprite_y = 2 * is_underground;

                        if (map[a][b] == Cell::ActivatedQuestionBlock) {
                            sprite_x = 6;
                            sprite_y++;
                        } else if (map[a][b] == Cell::Pipe) {
                            if (map_sketch.getPixel(a, b) == sf::Color(0, 182, 0)) {
                                sprite_y = 1;

                                if (map[a - 1][b] == Cell::Pipe) {
                                    sprite_x = 11;
                                } else {
                                    sprite_x = 10;
                                }
                            } else if (map_sketch.getPixel(a, b) == sf::Color(0, 146, 0)) {
                                sprite_y = 0;
                                if (map_sketch.getPixel(a - 1, b) == sf::Color(0, 146, 0)) {
                                    sprite_x = 11;
                                } else if (map_sketch.getPixel(1 + a, b) == sf::Color(0, 146, 0)) {
                                    sprite_x = 10;
                                } else {
                                    sprite_x = 10;
                                    if (map_sketch.getPixel(a, b - 1) == sf::Color(0, 146, 0)) {
                                        sprite_y = 3;
                                    } else {
                                        sprite_y = 2;
                                    }
                                }
                            } else if (map_sketch.getPixel(a, b) == sf::Color(0, 219, 0)) {
                                if (map_sketch.getPixel(1 + a, b) == sf::Color(0, 182, 0)) {
                                    sprite_x = 12;
                                } else {
                                    sprite_x = 11;
                                }
                                if (map_sketch.getPixel(a, b - 1) == sf::Color(0, 219, 0)) {
                                    sprite_y = 3;
                                } else {
                                    sprite_y = 2;
                                }
                            }
                        } else if (map[a][b] == Cell::Wall) {
                            if (map_sketch.getPixel(a, b) == sf::Color(0, 0, 0)) {
                                sprite_x = 2;
                            } else {
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

        //Drawing brick particles.
        if (!draw_background) {
            for (const Object& brick_particle : brick_particles) {
                cell_sprite.setPosition(brick_particle.x, brick_particle.y);
                cell_sprite.setTextureRect(sf::IntRect(
                    0.25f * CELL_SIZE, CELL_SIZE * (0.25f + 2 * is_underground), 0.5f * CELL_SIZE, 0.5f * CELL_SIZE
                    ));
                window.draw(cell_sprite);
            }
        }
    }

    void set_map_cell(uint16_t x, uint16_t y, const Cell& cell) {
        map[x][y] = cell;
    }

    void set_map_size(uint16_t new_size) {
        map.clear();
        map.resize(new_size);
    }

    void update_map_sketch(uint8_t current_level) {
        map_sketch.loadFromFile(std::format("Resources/Images/LevelSketch{}.png", current_level));
    }

    void update() {
        for (Object& question_block_coin : question_block_coins) {
            question_block_coin.vertical_speed += GRAVITY;
            question_block_coin.y += question_block_coin.vertical_speed;
        }

        for (Object& brick_particle : brick_particles) {
            brick_particle.vertical_speed += GRAVITY;
            brick_particle.x += brick_particle.horizontal_speed;
            brick_particle.y += brick_particle.vertical_speed;
        }

        unordered_erase_if(brick_particles, [](const Object& brick_particle) {
            return brick_particle.y >= SCREEN_HEIGHT;
        });
        unordered_erase_if(question_block_coins, [](const Object& question_block_coin) {
            return question_block_coin.vertical_speed >= 0;
        });

        coin_animation.update();
        question_block_animation.update();
    }

    std::vector<uint8_t> map_collision(const std::vector<Cell>& check_cells, const sf::FloatRect& hitbox) const {
        std::vector<uint8_t> output;

        for (short a = floor(hitbox.top / CELL_SIZE); a <= floor(
                 (ceil(hitbox.height + hitbox.top) - 1) / CELL_SIZE); a++) {
            output.push_back(0);

            for (short b = floor(hitbox.left / CELL_SIZE); b <= floor(
                     (ceil(hitbox.left + hitbox.width) - 1) / CELL_SIZE); b++) {
                if (0 <= b && b < map.size()) {
                    if (0 <= a && a < map[0].size()) {
                        if (check_cells.end() != std::ranges::find(check_cells, map[b][a])) {
                            //We're gonna return a vector of numbers. Each number is a binary representation of collisions in a single row.
                            output[a - floor(hitbox.top / CELL_SIZE)] += pow(
                                2, floor((ceil(hitbox.left + hitbox.width) - 1) / CELL_SIZE) - b);
                        }
                    }
                }
                //We're assuming that the map borders have walls.
                else if (check_cells.end() != std::ranges::find(check_cells, Cell::Wall)) {
                    output[a - floor(hitbox.top / CELL_SIZE)] += pow(
                        2, floor((ceil(hitbox.left + hitbox.width) - 1) / CELL_SIZE) - b);
                }
            }
        }

        return output;
    }

    std::vector<uint8_t> map_collision(const std::vector<Cell>& check_cells,
                                       std::vector<sf::Vector2i>& collision_cells,
                                       const sf::FloatRect& i_hitbox) const {
        std::vector<uint8_t> output;

        collision_cells.clear();

        for (short a = floor(i_hitbox.top / CELL_SIZE); a <= floor(
                 (ceil(i_hitbox.height + i_hitbox.top) - 1) / CELL_SIZE); a++) {
            output.push_back(0);

            for (short b = floor(i_hitbox.left / CELL_SIZE); b <= floor(
                     (ceil(i_hitbox.left + i_hitbox.width) - 1) / CELL_SIZE); b++) {
                if (0 <= b && b < map.size()) {
                    if (0 <= a && a < map[0].size()) {
                        if (check_cells.end() != std::find(check_cells.begin(), check_cells.end(), map[b][a])) {
                            //Since C++ doesn't support returning 2 vectors, we're gonna change the argument vector.
                            collision_cells.emplace_back(b, a);

                            output[a - floor(i_hitbox.top / CELL_SIZE)] += pow(
                                2, floor((ceil(i_hitbox.left + i_hitbox.width) - 1) / CELL_SIZE) - b);
                        }
                    }
                } else if (check_cells.end() != std::ranges::find(check_cells, Cell::Wall)) {
                    output[a - floor(i_hitbox.top / CELL_SIZE)] += pow(
                        2, floor((ceil(i_hitbox.left + i_hitbox.width) - 1) / CELL_SIZE) - b);
                }
            }
        }

        return output;
    }

    sf::Color get_map_sketch_pixel(uint16_t x, uint16_t y) const {
        return map_sketch.getPixel(x, y);
    }
};
