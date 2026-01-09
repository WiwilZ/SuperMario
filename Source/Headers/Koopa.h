#pragma once

#include <chrono>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Animation.h"
#include "Enemy.h"
#include "Global.h"
#include "Koopa.h"
#include "MapManager.h"
#include "Mario.h"


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

    uint16_t get_out_timer = KOOPA_GET_OUT_DURATION;

    sf::Texture texture;
    sf::Sprite sprite;

    Animation get_out_animation{CELL_SIZE, "Resources/Images/KoopaGetOut.png", KOOPA_GET_OUT_ANIMATION_SPEED};
    Animation walk_animation{CELL_SIZE, "Resources/Images/KoopaWalk.png", KOOPA_WALK_ANIMATION_SPEED};

public:
    Koopa(bool is_underground, float x, float y) :
        Enemy(x, y), is_underground(is_underground) {
        horizontal_speed = -KOOPA_SPEED;
        texture.loadFromFile(is_underground ? "Resources/Images/UndergroundKoopaShell.png" :
            "Resources/Images/KoopaShell.png");
        sprite.setTexture(texture);
        if (is_underground) {
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
        int posx = std::round(x);
        int posy = std::round(y);
        if (-CELL_SIZE < posy && posx > static_cast<int>(view_x) - CELL_SIZE
            && posx < SCREEN_WIDTH + view_x && posy < SCREEN_HEIGHT) {
            if (state > 0 || no_collision_dying) {
                if (!get_dead(false) && state == 1 && get_out_timer <= 0.25f * KOOPA_GET_OUT_DURATION) {
                    get_out_animation.set_position(posx, posy);
                    get_out_animation.draw(window);
                } else {
                    sprite.setPosition(posx, posy);
                    window.draw(sprite);
                }
            } else {
                walk_animation.set_flipped(flipped);
                walk_animation.set_position(posx, posy);
                walk_animation.draw(window);
            }
        }
    }

    void update(unsigned view_x, const std::vector<std::shared_ptr<Enemy>>& enemies,
                const MapManager& map_manager, Mario& mario) override {
        //I've already explained most of the code here in the Mario and Goomba classes.
        //I'm so bad at writing comments lol.

        if (-CELL_SIZE < y && x >= static_cast<int>(view_x) - CELL_SIZE - ENTITY_UPDATE_AREA
            && x < ENTITY_UPDATE_AREA + SCREEN_WIDTH + view_x && y < SCREEN_HEIGHT) {
            vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);

            if (!get_dead(false)) {
                sf::FloatRect hit_box = get_hit_box();

                hit_box.top += vertical_speed;

                auto collision = map_manager.map_collision(
                    {Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe, Cell::QuestionBlock, Cell::Wall}, hit_box
                    );

                if (!std::ranges::all_of(collision, [](const auto e) {
                    return e == 0;
                })) {
                    if (vertical_speed < 0) {
                        y = CELL_SIZE * (1 + std::floor((vertical_speed + y) / CELL_SIZE));
                    } else {
                        y = CELL_SIZE * (std::ceil((vertical_speed + y) / CELL_SIZE) - 1);
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

                collision = map_manager.map_collision(
                    {Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe, Cell::QuestionBlock, Cell::Wall}, hit_box
                    );

                if (!no_collision_dying && !std::ranges::all_of(collision, [](const auto e) {
                    return e == 0;
                })) {
                    if (horizontal_speed > 0) {
                        x = CELL_SIZE * (std::ceil((horizontal_speed + x) / CELL_SIZE) - 1);
                    } else {
                        x = CELL_SIZE * (1 + std::floor((horizontal_speed + x) / CELL_SIZE));
                    }
                    horizontal_speed *= -1;
                } else {
                    bool changed = false;

                    for (const auto& enemy : enemies) {
                        if (shared_from_this() != enemy && !enemy->get_dead(false)
                            && hit_box.intersects(enemy->get_hit_box())) {
                            if (state == 0) {
                                changed = true;
                                horizontal_speed *= -1;
                            } else if (state == 2) {
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
                            if (mario.get_vertical_speed() > 0) {
                                //We check the collision only once after we collide with Mario.
                                check_collision = false;
                                horizontal_speed = 0;
                                state = 1;
                                get_out_timer = KOOPA_GET_OUT_DURATION;
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            } else {
                                mario.die(false);
                            }
                            break;
                        case 1: //Start sliding.
                            check_collision = false;
                            state = 2;
                            //The direction is based on where Mario kicked Koopa.
                            if (mario.get_x() > x) {
                                horizontal_speed = -KOOPA_SHELL_SPEED;
                            } else {
                                horizontal_speed = KOOPA_SHELL_SPEED;
                            }
                            if (mario.get_vertical_speed() > 0) {
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            }
                            break;
                        default:
                            if (mario.get_vertical_speed() > 0) { //Stop sliding.
                                check_collision = false;
                                horizontal_speed = 0;
                                state = 1;
                                get_out_timer = KOOPA_GET_OUT_DURATION;
                                mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                            } else if (horizontal_speed > 0 && mario.get_x() > x
                                || horizontal_speed < 0 && mario.get_x() < x) {
                                //Don't stand in front of a sliding shell.
                                mario.die(false);
                            }
                        }
                    }
                } else {
                    //Once Mario stops intersecting with Koopa, start checking for collisions again.
                    check_collision = true;
                }

                if (horizontal_speed > 0) {
                    flipped = false;
                } else if (horizontal_speed < 0) {
                    flipped = true;
                }

                //Koopa can get out of his shell here.
                if (state == 1) {
                    get_out_timer--;

                    if (get_out_timer == 0) {
                        state = 0;
                        if (!flipped) {
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
