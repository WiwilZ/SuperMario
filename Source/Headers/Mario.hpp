#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Animation.hpp"
#include "Global.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"
#include "Mushroom.hpp"


class Mario {
    bool crouching = false;
    bool dead = false;
    bool flipped = false;
    bool on_ground = false;

    //Oh, this is a funny story!
    //So I was working on level 1-2 when I discovered a simple bug.
    //The bug was that when 2 Goombas stand SUPER close to each other and Mario jumps on them, one goomba dies and the other kills Mario.
    //This was happening because the first Goomba was setting Mario's vertical speed below 0 after dying.
    //Then the second Goomba checked the collision with Mario and saw that Mario wasn't squishing anymore and instead was going upwards.
    //So he was killing Mario.
    //And by adding this variable, I fixed the bug.
    //Hehe.
    float enemy_bounce_speed = 0;

    float x = 0;
    float y = 0;
    float horizontal_speed = 0;
    float vertical_speed = 0;

    //The longer we press the jump button, the higher Mario jumps. And we use this variable for that.
    uint8_t jump_timer = 0;
    //I was thinking about adding Fire Mario.
    //But I figured that it's gonna be too much work and I'm too lazy for that.
    //Maybe in the next part?
    uint8_t powerup_state = 0;

    uint16_t death_timer = MARIO_DEATH_DURATION;
    uint16_t growth_timer = 0;
    uint16_t invincible_timer = 0;

    std::vector<Mushroom> mushrooms;

    sf::Texture texture;
    sf::Sprite sprite;

    Animation big_walk_animation{CELL_SIZE, "Resources/Images/BigMarioWalk.png", MARIO_WALK_ANIMATION_SPEED};
    Animation walk_animation{CELL_SIZE, "Resources/Images/MarioWalk.png", MARIO_WALK_ANIMATION_SPEED};

public:
    Mario() {
        texture.loadFromFile("Resources/Images/MarioIdle.png");
        sprite.setTexture(texture);
    }

    bool get_dead() const {
        return dead;
    }

    float get_vertical_speed() const {
        return vertical_speed;
    }

    float get_x() const {
        return x;
    }

    void die(bool instant_death) {
        //Mario instantly dies and it doesn't matter if he's big or small.
        if (1 == instant_death) {
            dead = true;
            texture.loadFromFile(powerup_state == 0 ? "Resources/Images/MarioDeath.png" :
                                     "Resources/Images/BigMarioDeath.png");
        } else if (0 == growth_timer && 0 == invincible_timer) { //Mario dies, unless he's big.
            if (0 == powerup_state) {
                dead = true;
                texture.loadFromFile("Resources/Images/MarioDeath.png");
            } else {
                powerup_state = 0;
                invincible_timer = MARIO_INVINCIBILITY_DURATION;
                if (!crouching) {
                    y += CELL_SIZE;
                } else {
                    crouching = false;
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        //When Mario is invincible, his sprite will blink.
        if (0 == invincible_timer / MARIO_BLINKING % 2) {
            bool draw_sprite = true;
            //When Mario is growing, his sprite will switch between being big and small.
            const bool draw_big = 0 == growth_timer / MARIO_BLINKING % 2;

            sprite.setPosition(round(x), round(y));

            if (!dead) {
                if (0 < powerup_state) {
                    if (crouching) {
                        texture.loadFromFile(draw_big ? "Resources/Images/BigMarioCrouch.png" :
                                                 "Resources/Images/MarioIdle.png");
                    } else if (!on_ground) {
                        if (!draw_big) {
                            sprite.setPosition(round(x), CELL_SIZE + round(y));
                            texture.loadFromFile("Resources/Images/MarioJump.png");
                        } else {
                            texture.loadFromFile("Resources/Images/BigMarioJump.png");
                        }
                    } else {
                        if (0 == horizontal_speed) {
                            if (!draw_big) {
                                sprite.setPosition(round(x), CELL_SIZE + round(y));
                                texture.loadFromFile("Resources/Images/MarioIdle.png");
                            } else {
                                texture.loadFromFile("Resources/Images/BigMarioIdle.png");
                            }
                        } else if ((0 < horizontal_speed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
                                sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) ||
                            (0 > horizontal_speed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
                                sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) {
                            if (!draw_big) {
                                sprite.setPosition(round(x), CELL_SIZE + round(y));
                                texture.loadFromFile("Resources/Images/MarioBrake.png");
                            } else {
                                texture.loadFromFile("Resources/Images/BigMarioBrake.png");
                            }
                        } else {
                            draw_sprite = false;

                            if (!draw_big) {
                                walk_animation.set_flipped(flipped);
                                walk_animation.set_position(round(x), CELL_SIZE + round(y));
                                walk_animation.draw(window);
                            } else {
                                big_walk_animation.set_flipped(flipped);
                                big_walk_animation.set_position(round(x), round(y));
                                big_walk_animation.draw(window);
                            }
                        }
                    }
                } else if (!on_ground) {
                    texture.loadFromFile("Resources/Images/MarioJump.png");
                } else {
                    if (0 == horizontal_speed) {
                        texture.loadFromFile("Resources/Images/MarioIdle.png");
                    } else if ((0 < horizontal_speed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) ||
                        (0 > horizontal_speed && !sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) {
                        texture.loadFromFile("Resources/Images/MarioBrake.png");
                    } else {
                        draw_sprite = false;
                        walk_animation.set_flipped(flipped);
                        walk_animation.set_position(round(x), round(y));
                        walk_animation.draw(window);
                    }
                }
            }

            if (draw_sprite) {
                if (!flipped) {
                    sprite.setTextureRect(sf::IntRect(0, 0, texture.getSize().x, texture.getSize().y));
                } else {
                    sprite.setTextureRect(sf::IntRect(texture.getSize().x, 0, -static_cast<int>(texture.getSize().x),
                                                      texture.getSize().y));
                }
                window.draw(sprite);
            }
        }
    }

    void draw_mushrooms(unsigned view_x, sf::RenderWindow& window) {
        for (Mushroom& mushroom : mushrooms) {
            mushroom.draw(view_x, window);
        }
    }

    void reset() {
        crouching = false;
        dead = false;
        flipped = false;
        on_ground = false;

        enemy_bounce_speed = 0;
        x = 0;
        y = 0;
        horizontal_speed = 0;
        vertical_speed = 0;

        jump_timer = 0;
        powerup_state = 0;

        //If it wasn't for this, everything would be zerfect! (see what I did there?)
        //...
        //...
        //...
        //Yeah, I agree. That was bad.
        death_timer = MARIO_DEATH_DURATION;
        growth_timer = 0;
        invincible_timer = 0;

        mushrooms.clear();

        texture.loadFromFile("Resources/Images/MarioIdle.png");

        sprite.setTexture(texture);

        big_walk_animation.set_animation_speed(MARIO_WALK_ANIMATION_SPEED);
        big_walk_animation.set_flipped(0);

        walk_animation.set_animation_speed(MARIO_WALK_ANIMATION_SPEED);
        walk_animation.set_flipped(0);
    }

    void set_position(float x, float y) {
        this->x = x;
        this->y = y;
    }

    void set_vertical_speed(float speed) {
        enemy_bounce_speed = speed;
    }

    void update(unsigned view_x, MapManager& map_manager) {
        //We make Mario bounce after updating all the enemies to prevent a bug (Go to Mario.hpp for explanation).
        if (0 != enemy_bounce_speed) {
            vertical_speed = enemy_bounce_speed;

            enemy_bounce_speed = 0;
        }

        for (Mushroom& mushroom : mushrooms) {
            mushroom.update(view_x, map_manager);
        }

        if (!dead) {
            bool moving = false;

            //So basically, the map_collision function returns a vector of numbers. Each number is a binary representation of the collisions in a single row. And we're storing that vector in this vector.
            std::vector<uint8_t> collision;

            //Oh yeah, the map_collision function can also return the coordinates of cells intersecting the hitbox. We need that too.
            std::vector<sf::Vector2i> cells;

            sf::FloatRect hit_box = get_hit_box();

            on_ground = false;

            if (!crouching) {
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    moving = true;
                    horizontal_speed = std::max(horizontal_speed - MARIO_ACCELERATION, -MARIO_WALK_SPEED);
                }

                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    moving = true;
                    horizontal_speed = std::min(MARIO_ACCELERATION + horizontal_speed, MARIO_WALK_SPEED);
                }
            }

            if (!moving) {
                if (0 < horizontal_speed) {
                    horizontal_speed = std::max<float>(0, horizontal_speed - MARIO_ACCELERATION);
                } else if (0 > horizontal_speed) {
                    horizontal_speed = std::min<float>(0, MARIO_ACCELERATION + horizontal_speed);
                }
            }

            if (0 < powerup_state) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::C) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                    if (!crouching) {
                        crouching = true;
                        y += CELL_SIZE;
                    }
                } else if (crouching) {
                    hit_box.height += CELL_SIZE;
                    hit_box.top -= CELL_SIZE;

                    //Making sure we can stand up without hitting anything.
                    collision = map_manager.map_collision({
                                                              Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                              Cell::QuestionBlock, Cell::Wall
                                                          }, hit_box);

                    if (std::ranges::all_of(collision, [](const uint8_t e) {
                        return 0 == e;
                    })) {
                        crouching = false;
                        y -= CELL_SIZE;
                    } else {
                        collision = map_manager.map_collision({
                                                                  Cell::ActivatedQuestionBlock, Cell::Pipe,
                                                                  Cell::QuestionBlock, Cell::Wall
                                                              }, hit_box);

                        //But if it happens to be bricks, we'll destroy them.
                        if (std::ranges::all_of(collision, [](const uint8_t e) {
                            return 0 == e;
                        })) {
                            crouching = false;
                            y -= CELL_SIZE;

                            map_manager.map_collision({Cell::Brick}, cells, hit_box);

                            for (const sf::Vector2i& cell : cells) {
                                map_manager.set_map_cell(cell.x, cell.y, Cell::Empty);
                                map_manager.add_brick_particles(CELL_SIZE * cell.x, CELL_SIZE * cell.y);
                            }
                        }
                    }
                }
            }

            hit_box = get_hit_box();
            hit_box.left += horizontal_speed;

            collision = map_manager.map_collision({
                                                      Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                      Cell::QuestionBlock, Cell::Wall
                                                  }, hit_box);

            if (!std::ranges::all_of(collision, [](const uint8_t e) {
                return 0 == e;
            })) {
                moving = false;

                if (0 < horizontal_speed) {
                    x = CELL_SIZE * (ceil((horizontal_speed + x) / CELL_SIZE) - 1);
                } else if (0 > horizontal_speed) {
                    x = CELL_SIZE * (1 + floor((horizontal_speed + x) / CELL_SIZE));
                }

                horizontal_speed = 0;
            } else {
                x += horizontal_speed;
            }

            hit_box = get_hit_box();
            hit_box.top++;

            collision = map_manager.map_collision({
                                                      Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                      Cell::QuestionBlock, Cell::Wall
                                                  }, hit_box);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                if (0 == vertical_speed && !std::ranges::all_of(collision,
                                                                [](const uint8_t e) {
                                                                    return 0 == e;
                                                                })) {
                    vertical_speed = MARIO_JUMP_SPEED;
                    jump_timer = MARIO_JUMP_TIMER;
                } else if (0 < jump_timer) { //The longer we press the jump button, the higher Mario jumps.
                    vertical_speed = MARIO_JUMP_SPEED;
                    jump_timer--;
                } else {
                    vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);
                }
            } else {
                vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);
                jump_timer = 0;
            }

            hit_box = get_hit_box();
            hit_box.top += vertical_speed;

            collision = map_manager.map_collision({
                                                      Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                      Cell::QuestionBlock, Cell::Wall
                                                  }, hit_box);

            if (!std::ranges::all_of(collision, [](const uint8_t e) {
                return 0 == e;
            })) {
                if (0 > vertical_speed) {
                    //Destroying bricks!!!!
                    if (!crouching && 0 < powerup_state) {
                        map_manager.map_collision({Cell::Brick}, cells, hit_box);

                        for (const sf::Vector2i& cell : cells) {
                            map_manager.set_map_cell(cell.x, cell.y, Cell::Empty);
                            map_manager.add_brick_particles(CELL_SIZE * cell.x, CELL_SIZE * cell.y);
                        }
                    }

                    map_manager.map_collision({Cell::QuestionBlock}, cells, hit_box);

                    //Activating question blocks!!!!
                    for (const sf::Vector2i& cell : cells) {
                        map_manager.set_map_cell(cell.x, cell.y, Cell::ActivatedQuestionBlock);

                        //It can be either a mushroom or a coin, depending on the color of the pixel in the sketch.
                        if (sf::Color(255, 73, 85) == map_manager.get_map_sketch_pixel(cell.x, cell.y)) {
                            mushrooms.emplace_back(CELL_SIZE * cell.x, CELL_SIZE * cell.y);
                        } else {
                            map_manager.add_question_block_coin(CELL_SIZE * cell.x, CELL_SIZE * cell.y);
                        }
                    }

                    y = CELL_SIZE * (1 + floor((vertical_speed + y) / CELL_SIZE));
                } else if (0 < vertical_speed) {
                    y = CELL_SIZE * (ceil((vertical_speed + y) / CELL_SIZE) - 1);
                }

                jump_timer = 0;
                vertical_speed = 0;
            } else {
                y += vertical_speed;
            }

            if (0 == horizontal_speed) {
                if (moving) {
                    flipped = !flipped;
                }
            } else if (0 < horizontal_speed) {
                flipped = false;
            } else if (0 > horizontal_speed) {
                flipped = true;
            }

            hit_box = get_hit_box();
            hit_box.top++;

            collision = map_manager.map_collision({
                                                      Cell::ActivatedQuestionBlock, Cell::Brick, Cell::Pipe,
                                                      Cell::QuestionBlock, Cell::Wall
                                                  }, hit_box);

            if (!std::ranges::all_of(collision, [](const uint8_t e) {
                return e == 0;
            })) {
                on_ground = true;
            }

            for (Mushroom& mushroom : mushrooms) {
                //Mushroom eating and becoming BIG, STRONG, MASCULINE!!!!
                if (1 == get_hit_box().intersects(mushroom.get_hit_box())) {
                    mushroom.set_dead(true);
                    if (0 == powerup_state) {
                        powerup_state = 1;
                        growth_timer = MARIO_GROWTH_DURATION;
                        y -= CELL_SIZE;
                    }
                }
            }

            if (0 < invincible_timer) {
                invincible_timer--;
            }

            hit_box = get_hit_box();

            map_manager.map_collision({Cell::Coin}, cells, hit_box);

            //Collecting coins.
            for (const auto& cell : cells) {
                map_manager.set_map_cell(cell.x, cell.y, Cell::Empty);
            }

            if (0 < growth_timer) {
                growth_timer--;
            }

            if (y >= SCREEN_HEIGHT - get_hit_box().height) {
                die(true);
            }

            if (0 == powerup_state) {
                walk_animation.set_animation_speed(
                    MARIO_WALK_ANIMATION_SPEED * MARIO_WALK_SPEED / abs(horizontal_speed));
                walk_animation.update();
            } else {
                big_walk_animation.set_animation_speed(
                    MARIO_WALK_ANIMATION_SPEED * MARIO_WALK_SPEED / abs(horizontal_speed));
                big_walk_animation.update();
            }
        } else {
            if (0 == death_timer) {
                vertical_speed = std::min(GRAVITY + vertical_speed, MAX_VERTICAL_SPEED);
                y += vertical_speed;
            } else if (1 == death_timer) {
                vertical_speed = MARIO_JUMP_SPEED;
            }

            death_timer = std::max(0, death_timer - 1);
        }

        //Deleting mushrooms from the vector.
        std::erase_if(mushrooms, [](const Mushroom& mushroom) {
            return mushroom.get_dead();
        });
    }

    sf::FloatRect get_hit_box() const {
        //The hitbox will be small if Mario is small or crouching.
        float height = CELL_SIZE;
        if (!crouching && powerup_state != 0) {
            height *= 2;
        }
        return {x, y, CELL_SIZE, height};
    }
};
