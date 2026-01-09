#pragma once

#include <array>
#include <chrono>
#include <vector>


constexpr float BRICK_PARTICLE_SPEED = 4;
//A coin jumping out of the question block.
constexpr float COIN_JUMP_SPEED = -6;
constexpr float GOOMBA_SPEED = 1;
constexpr float GRAVITY = 0.25f;
constexpr float KOOPA_SHELL_SPEED = 4;
constexpr float KOOPA_SPEED = 1;
constexpr float MARIO_ACCELERATION = 0.25f;
constexpr float MARIO_JUMP_SPEED = -4;
constexpr float MARIO_WALK_SPEED = 2;
constexpr float MAX_VERTICAL_SPEED = 8;
constexpr float MUSHROOM_SPEED = 1;

constexpr uint8_t CELL_SIZE = 16;
//We use this constant when Mario is growing or when he's invincible.
constexpr uint8_t MARIO_BLINKING = 4;
//Lemme explain:
//1) Mario gets hit by an enemy.
//2) Mario looks at you like "Oh no! Me lost! Mama mia!" and pauses for this amount.
//3) Mario bounces off the screen.
constexpr uint8_t MARIO_DEATH_DURATION = 32;
constexpr uint8_t MARIO_GROWTH_DURATION = 64;
constexpr uint8_t MARIO_INVINCIBILITY_DURATION = 128;
//The longer we press the jump button, the higher Mario jumps.
constexpr uint8_t MARIO_JUMP_TIMER = 8;
constexpr uint8_t SCREEN_RESIZE = 2;

constexpr uint16_t COIN_ANIMATION_SPEED = 4;
//Since we deactivate everything outside the view, we need to have a safe area around the view where objects start updating.
constexpr uint16_t ENTITY_UPDATE_AREA = 64;
constexpr uint16_t GOOMBA_DEATH_DURATION = 32;
//Why do I have so many animation speed constants? I use them only once.
constexpr uint16_t GOOMBA_WALK_ANIMATION_SPEED = 32;
constexpr uint16_t KOOPA_GET_OUT_ANIMATION_SPEED = 4;
constexpr uint16_t KOOPA_GET_OUT_DURATION = 512;
constexpr uint16_t KOOPA_WALK_ANIMATION_SPEED = 32;
constexpr uint16_t MARIO_WALK_ANIMATION_SPEED = 4;
constexpr uint16_t QUESTION_BLOCK_ANIMATION_SPEED = 8;
constexpr uint16_t SCREEN_HEIGHT = 240;
constexpr uint16_t SCREEN_WIDTH = 320;

constexpr std::chrono::microseconds FRAME_DURATION(16667);


enum class Cell : uint8_t {
    ActivatedQuestionBlock,
    Brick,
    Coin,
    Empty,
    Pipe,
    QuestionBlock,
    Wall
};


//We use this for brick particles.
//Damn, I should've called it "Particle"! I'm so dumb!
struct Object {
    float x;
    float y;
    float horizontal_speed;
    float vertical_speed;

    Object(float x, float y, float horizontal_speed = 0, float vertical_speed = 0) :
        x(x), y(y), horizontal_speed(horizontal_speed), vertical_speed(vertical_speed) {}
};


using Map = std::vector<std::array<Cell, SCREEN_HEIGHT / CELL_SIZE>>;
