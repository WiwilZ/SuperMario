#pragma once

#include <array>
#include <chrono>
#include <cmath>
#include <memory>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "Animation.hpp"
#include "Enemy.hpp"
#include "Global.hpp"
#include "Goomba.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"


class Goomba : public Enemy {
    //This is kinda hard to explain.
    //Okay, so both Goomba and Koopa can die from Koopa's shell.
    //And when they do, they bounce of the screen.
    //So we need a variable that lets us know "Hey! I hit the shell. I'm dying right now from hitting that shell. That damn shell!".
    bool no_collision_dying = false;
    //The underground Goomba looks different than the regular Goomba.
    bool is_underground;

    uint16_t death_timer;

    sf::Texture texture;
    sf::Sprite sprite;

    Animation walk_animation;

public:
    Goomba(bool is_underground, float x, float y) :
        Enemy(x, y),
        is_underground(is_underground),
        death_timer(GOOMBA_DEATH_DURATION),
        walk_animation(CELL_SIZE, "Resources/Images/GoombaWalk.png", GOOMBA_WALK_ANIMATION_SPEED) {
        horizontal_speed = -GOOMBA_SPEED;
        if (!is_underground) {
            texture.loadFromFile("Resources/Images/GoombaDeath0.png");
        } else {
            texture.loadFromFile("Resources/Images/UndergroundGoombaDeath0.png");
            walk_animation.set_texture_location("Resources/Images/UndergroundGoombaWalk.png");
        }
    }

    bool get_dead(bool deletion) const override {
        if (deletion) {
            return dead;
        }
        return dead || no_collision_dying || GOOMBA_DEATH_DURATION > death_timer;
    }

    void die(uint8_t death_type) override {
        switch (death_type) {
        case 0: //Instant death. Setting dead to 1 will immediately delete the object.
            dead = true;
            break;
        case 1: //Goomba is squished by Mario.
            if (!no_collision_dying) {
                death_timer--;
            }
            break;
        case 2:
            if (GOOMBA_DEATH_DURATION == death_timer) {
                //Goomba dies from Koopa's shell.
                no_collision_dying = true;
                vertical_speed = 0.5f * MARIO_JUMP_SPEED;
                texture.loadFromFile(is_underground ? "Resources/Images/UndergroundGoombaDeath1.png" :
                                         "Resources/Images/GoombaDeath1.png");
            }
        }
    }

    void draw(unsigned view_x, sf::RenderWindow& window) override {
        //Making sure we don't draw Goomba outside the view.
        if (-CELL_SIZE < round(y) && round(x) > static_cast<int>(view_x) - CELL_SIZE && round(x) < SCREEN_WIDTH +
            view_x && round(y) < SCREEN_HEIGHT) {
            if (no_collision_dying || GOOMBA_DEATH_DURATION > death_timer) {
                sprite.setPosition(round(x), round(y));
                sprite.setTexture(texture);
                window.draw(sprite);
            } else {
                walk_animation.set_position(round(x), round(y));
                walk_animation.draw(window);
            }
        }
    }

    void update(unsigned view_x, const std::vector<std::shared_ptr<Enemy>>& enemies,
                const MapManager& map_manager, Mario& mario) override {
        //I've already explained most of the code here in the Mario class.
        //I know it's bad to write the same code multiple times.
        //But I kinda don't care.

        //Making sure we don't update Goomba outside the view.
        if (-CELL_SIZE < y && x >= static_cast<int>(view_x) - CELL_SIZE - ENTITY_UPDATE_AREA && x < ENTITY_UPDATE_AREA
            + SCREEN_WIDTH + view_x && y < SCREEN_HEIGHT) {
            sf::FloatRect hit_box = get_hit_box();

            vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);

            hit_box.top += vertical_speed;

            std::vector<uint8_t> collision = map_manager.map_collision({
                                                                           Cell::ActivatedQuestionBlock, Cell::Brick,
                                                                           Cell::Pipe,
                                                                           Cell::QuestionBlock, Cell::Wall
                                                                       }, hit_box);

            if (!no_collision_dying) {
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

                    //Here we're making sure that when Goomba falls on another enemy, they don't intersect.
                    if (!get_dead(false)) {
                        for (const auto& enemy : enemies) {
                            if (shared_from_this() != enemy && !enemy->get_dead(false) && hit_box.
                                intersects(enemy->get_hit_box())) {
                                changed = true;
                                if (0 > vertical_speed) {
                                    y = enemy->get_hit_box().height + enemy->get_hit_box().top;
                                } else {
                                    y = enemy->get_hit_box().top - CELL_SIZE;
                                }
                                vertical_speed = 0;
                                break;
                            }
                        }
                    }

                    if (!changed) {
                        y += vertical_speed;
                    }
                }

                if (!get_dead(false)) {
                    hit_box = get_hit_box();
                    hit_box.left += horizontal_speed;

                    collision = map_manager.map_collision({
                                                              Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                              Cell::QuestionBlock, Cell::Wall
                                                          }, hit_box);

                    if (!std::ranges::all_of(collision, [](const auto e) {
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

                        //Changing direction when colliding with another enemy.
                        for (const auto& enemy : enemies) {
                            if (shared_from_this() != enemy && !enemy->get_dead(false) && hit_box.
                                intersects(enemy->get_hit_box())) {
                                changed = true;
                                horizontal_speed *= -1;
                                break;
                            }
                        }

                        if (!changed) {
                            x += horizontal_speed;
                        }
                    }

                    if (!mario.get_dead() && get_hit_box().intersects(mario.get_hit_box())) {
                        //If Mario is falling...
                        if (0 < mario.get_vertical_speed()) {
                            //... then we get squished.
                            die(1);
                            mario.set_vertical_speed(0.5f * MARIO_JUMP_SPEED);
                        } else {
                            //Otherwise, kill Mario.
                            mario.die(false);
                        }
                    }

                    walk_animation.update();
                } else if (GOOMBA_DEATH_DURATION > death_timer) {
                    if (0 < death_timer) {
                        death_timer--;
                    } else {
                        die(0);
                    }
                }
            } else {
                y += vertical_speed;
            }
        }

        if (SCREEN_HEIGHT <= y) {
            die(0);
        }
    }
};
