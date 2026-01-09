#pragma once

#include <chrono>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Global.hpp"
#include "MapManager.hpp"
#include "Mushroom.hpp"


class Mushroom {
    bool dead = false;
    float x;
    float y;
    int8_t horizontal_direction = 1;
    float vertical_speed = 0;

    //Before the mushroom begins to move, it needs to get out of the question block.
    uint16_t start_y;

    sf::Texture texture;
    sf::Sprite sprite;

public:
    Mushroom(float x, float y) : x(x), y(y), start_y(y) {
        texture.loadFromFile("Resources/Images/Mushroom.png");
    }

    bool get_dead() const {
        return dead;
    }

    void draw(unsigned view_x, sf::RenderWindow& window) {
        if (-CELL_SIZE < round(y) && round(x) > static_cast<int>(view_x) - CELL_SIZE && round(x) < SCREEN_WIDTH +
            view_x && round(y) < SCREEN_HEIGHT) {
            sprite.setPosition(round(x), round(y));
            sprite.setTexture(texture);
            window.draw(sprite);
        }
    }

    void set_dead(bool value) {
        dead = value;
    }

    void update(unsigned view_x, const MapManager& map_manager) {
        //Mushrooms behave the same way Goombas do so go check out the Goomba class.
        if (!dead) {
            if (-CELL_SIZE < y && x >= static_cast<int>(view_x) - CELL_SIZE - ENTITY_UPDATE_AREA && x <
                ENTITY_UPDATE_AREA + SCREEN_WIDTH + view_x && y < SCREEN_HEIGHT) {
                if (y > start_y - CELL_SIZE) {
                    y -= MUSHROOM_SPEED;
                    if (y <= start_y - CELL_SIZE) {
                        y = start_y - CELL_SIZE;
                        start_y = 2 * SCREEN_HEIGHT;
                    }
                } else {
                    vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);

                    std::vector<uint8_t> collision = map_manager.map_collision(
                        {Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe, Cell::QuestionBlock, Cell::Wall},
                        sf::FloatRect(x, vertical_speed + y, CELL_SIZE, CELL_SIZE));

                    if (!std::ranges::all_of(collision, [](const uint8_t e) {
                        return 0 == e;
                    })) {
                        if (0 > vertical_speed) {
                            y = CELL_SIZE * (1 + floor((vertical_speed + y) / CELL_SIZE));
                        } else {
                            y = CELL_SIZE * (ceil((vertical_speed + y) / CELL_SIZE) - 1);
                        }
                        vertical_speed = 0;
                    } else {
                        y += vertical_speed;
                    }

                    sf::FloatRect horizontal_hit_box{
                        MUSHROOM_SPEED * horizontal_direction + x, y, CELL_SIZE, CELL_SIZE
                    };

                    collision = map_manager.map_collision({
                                                              Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                              Cell::QuestionBlock, Cell::Wall
                                                          }, horizontal_hit_box);

                    if (!std::ranges::all_of(collision, [](const uint8_t e) {
                        return 0 == e;
                    })) {
                        if (0 < horizontal_direction) {
                            x = CELL_SIZE * (ceil((MUSHROOM_SPEED * horizontal_direction + x) / CELL_SIZE) - 1);
                        } else {
                            x = CELL_SIZE * (1 + floor((MUSHROOM_SPEED * horizontal_direction + x) / CELL_SIZE));
                        }
                        horizontal_direction *= -1;
                    } else {
                        x += MUSHROOM_SPEED * horizontal_direction;
                    }
                }
            }

            if (SCREEN_HEIGHT <= y) {
                dead = 1;
            }
        }
    }

    sf::FloatRect get_hit_box() const {
        return {x, y, CELL_SIZE, CELL_SIZE};
    }
};
