#pragma once

#include <chrono>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Animation.hpp"
#include "Enemy.hpp"
#include "Global.hpp"
#include "Koopa.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"


class Koopa : public Enemy {
    //We'll use this to make sure that we check the collision with Mario only once when he collides with us.
    bool check_collision = true;
    bool flipped = true;
    //Look at the declaration of the Goomba class. I don't wanna explain this again.
    //I know I can just copy and paste, but I want you to do some work too, y'know.
    //I mean, why do I have to do everything for you?
    //Why do I have to waste my time copying and pasting 4 lines of comments when you can just open the file I told you to open?
    //Are you better than me?
    bool no_collision_dying = false;
    bool is_underground;

    //0 - Koopa walk
    //1 - Koopa shell
    //2 - Koopa shell slide
    uint8_t state = 0;

    uint16_t get_out_timer;

    sf::Texture texture;
    sf::Sprite sprite;

    Animation get_out_animation;
    Animation walk_animation;

public:
    Koopa(bool is_underground, float x, float y) :
        Enemy(x, y),
        is_underground(is_underground),
        get_out_timer(KOOPA_GET_OUT_DURATION),
        get_out_animation(CELL_SIZE, "Resources/Images/KoopaGetOut.png", KOOPA_GET_OUT_ANIMATION_SPEED),
        walk_animation(CELL_SIZE, "Resources/Images/KoopaWalk.png", KOOPA_WALK_ANIMATION_SPEED) {
        horizontal_speed = -KOOPA_SPEED;
        if (!is_underground) {
            texture.loadFromFile("Resources/Images/KoopaShell.png");
        } else {
            texture.loadFromFile("Resources/Images/UndergroundKoopaShell.png");
            get_out_animation.set_texture_location("Resources/Images/UndergroundKoopaGetOut.png");
            walk_animation.set_texture_location("Resources/Images/UndergroundKoopaWalk.png");
        }
    }

    bool get_dead(bool deletion) const override {
        if (deletion) {
            return dead;
        }
        return dead || no_collision_dying;
    }

    void die(uint8_t death_type) override {
        switch (death_type) {
        case 0: //Instant death.
            dead = true;
            break;
        case 2: //Dying from Koopa's shell.
            no_collision_dying = true;
            vertical_speed = 0.5f * MARIO_JUMP_SPEED;
            texture.loadFromFile(is_underground ? "Resources/Images/UndergroundKoopaDeath.png" :
                                     "Resources/Images/KoopaDeath.png");
        }
    }

    void draw(unsigned view_x, sf::RenderWindow& window) override {
        if (-CELL_SIZE < round(y) && round(x) > static_cast<int>(view_x) - CELL_SIZE && round(x) < SCREEN_WIDTH +
            view_x && round(y) < SCREEN_HEIGHT) {
            if (0 < state || no_collision_dying) {
                if (!get_dead(false) && 1 == state && get_out_timer <= 0.25f * KOOPA_GET_OUT_DURATION) {
                    get_out_animation.set_position(round(x), round(y));
                    get_out_animation.draw(window);
                } else {
                    sprite.setPosition(round(x), round(y));
                    sprite.setTexture(texture);
                    window.draw(sprite);
                }
            } else {
                walk_animation.set_flipped(flipped);
                walk_animation.set_position(round(x), round(y));
                walk_animation.draw(window);
            }
        }
    }

    void update(unsigned view_x, const std::vector<std::shared_ptr<Enemy>>& enemies,
                const MapManager& map_manager, Mario& mario) override {
        //I've already explained most of the code here in the Mario and Goomba classes.
        //I'm so bad at writing comments lol.

        if (-CELL_SIZE < y && x >= static_cast<int>(view_x) - CELL_SIZE - ENTITY_UPDATE_AREA && x < ENTITY_UPDATE_AREA
            + SCREEN_WIDTH + view_x && y < SCREEN_HEIGHT) {
            vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);

            if (!get_dead(false)) {
                sf::FloatRect hit_box = get_hit_box();

                hit_box.top += vertical_speed;

                std::vector<uint8_t> collision = map_manager.map_collision({
                                                                               Cell::ActivatedQuestionBlock,
                                                                               Cell::Brick,
                                                                               Cell::Pipe,
                                                                               Cell::QuestionBlock, Cell::Wall
                                                                           }, hit_box);

                if (!std::ranges::all_of(collision, [](const auto e) {
                    return e == 0;
                })) {
                    if (0 > vertical_speed) {
                        y = CELL_SIZE * (1 + floor((vertical_speed + y) / CELL_SIZE));
                    } else {
                        y = CELL_SIZE * (ceil((vertical_speed + y) / CELL_SIZE) - 1);
                    }
                    vertical_speed = 0;
                } else {
                    bool changed = false;

                    for (const auto& enemy : enemies) {
                        if (shared_from_this() != enemy && !enemy->get_dead(false) && hit_box.
                            intersects(enemy->get_hit_box())) {
                            changed = true;
                            if (0 > vertical_speed) {
                                y = enemy->get_hit_box().top + enemy->get_hit_box().height;
                            } else {
                                y = enemy->get_hit_box().top - CELL_SIZE;
                            }
                            vertical_speed = 0;
                            break;
                        }
                    }

                    if (!changed) {
                        y += vertical_speed;
                    }
                }

                hit_box = get_hit_box();
                hit_box.left += horizontal_speed;

                collision = map_manager.map_collision({
                                                          Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                          Cell::QuestionBlock, Cell::Wall
                                                      }, hit_box);

                if (!no_collision_dying && !std::ranges::all_of(collision,
                                                                [](const auto e) {
                                                                    return e == 0;
                                                                })) {
                    if (0 < horizontal_speed) {
                        x = CELL_SIZE * (ceil((horizontal_speed + x) / CELL_SIZE) - 1);
                    } else {
                        x = CELL_SIZE * (1 + floor((horizontal_speed + x) / CELL_SIZE));
                    }
                    horizontal_speed *= -1;
                } else {
                    bool changed = false;

                    for (const auto& enemy : enemies) {
                        if (shared_from_this() != enemy && !enemy->get_dead(false) && hit_box.
                            intersects(enemy->get_hit_box())) {
                            if (0 == state) {
                                changed = true;
                                horizontal_speed *= -1;
                            } else if (2 == state) {
                                enemy->die(2);
                            }
                            break;
                        }
                    }

                    if (!changed) {
                        x += horizontal_speed;
                    }
                }

                if (!mario.get_dead() && get_hit_box().intersects(mario.get_hit_box())) {
                    if (check_collision) {
                        switch (state) {
                        case 0: //Turning into a shell.
                            if (0 < mario.get_vertical_speed()) {
                                //We check the collision only once after we collide with Mario.
                                check_collision = false;
                                horizontal_speed = 0;
                                state = 1;
                                get_out_timer = KOOPA_GET_OUT_DURATION;
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            } else {
                                mario.die(0);
                            }
                            break;
                        case 1: //Start sliding.
                            check_collision = false;
                            state = 2;
                            //The direction is based on where Mario kicked Koopa.
                            if (x < mario.get_x()) {
                                horizontal_speed = -KOOPA_SHELL_SPEED;
                            } else {
                                horizontal_speed = KOOPA_SHELL_SPEED;
                            }
                            if (0 < mario.get_vertical_speed()) {
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            }
                            break;
                        default:
                            if (0 < mario.get_vertical_speed()) { //Stop sliding.
                                check_collision = false;
                                horizontal_speed = 0;
                                state = 1;
                                get_out_timer = KOOPA_GET_OUT_DURATION;
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            } else if ((0 < horizontal_speed && x < mario.get_x()) ||
                                (0 > horizontal_speed && x > mario.get_x())) {
                                //Don't stand in front of a sliding shell.
                                mario.die(false);
                            }
                        }
                    }
                } else {
                    //Once Mario stops intersecting with Koopa, start checking for collisions again.
                    check_collision = true;
                }

                if (0 < horizontal_speed) {
                    flipped = 0;
                } else if (0 > horizontal_speed) {
                    flipped = 1;
                }

                //Koopa can get out of his shell here.
                if (1 == state) {
                    get_out_timer--;

                    if (0 == get_out_timer) {
                        state = 0;
                        if (0 == flipped) {
                            horizontal_speed = KOOPA_SPEED;
                        } else {
                            horizontal_speed = -KOOPA_SPEED;
                        }
                    } else {
                        get_out_animation.update();
                    }
                }

                walk_animation.update();
            } else {
                y += vertical_speed;
            }
        }

        if (SCREEN_HEIGHT <= y) {
            die(0);
        }
    }
};
