#pragma once

#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>


class Animation {
    bool flipped = false;

    //We'll increment the current_frame as soon as this iterator reaches the animation_speed.
    uint16_t animation_iterator = 0;
    //After how many frames do we change the current_frame?
    uint16_t animation_speed;
    uint16_t current_frame = 0;
    uint16_t frame_width;
    uint16_t total_frames;

    sf::Texture texture;
    sf::Sprite sprite;

public:
    Animation(uint16_t frame_width, const std::string& texture_path, uint16_t animation_speed = 1) :
        animation_speed(std::max<uint16_t>(1, animation_speed)),
        frame_width(frame_width) {
        texture.loadFromFile(texture_path);
        total_frames = texture.getSize().x / frame_width;
    }

    void draw(sf::RenderWindow& window) {
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(frame_width * (flipped + current_frame), 0, flipped ? -frame_width : frame_width, texture.getSize().y));
        window.draw(sprite);
    }

    void set_animation_speed(uint16_t speed) {
        animation_speed = std::max<uint16_t>(1, speed);
    }

    void set_flipped(bool value) {
        flipped = value;
    }

    void set_position(uint16_t x, uint16_t y) {
        sprite.setPosition(x, y);
    }

    void set_texture_location(const std::string& texture_path) {
        texture.loadFromFile(texture_path);
    }

    void update() {
        while (animation_iterator >= animation_speed) {
            animation_iterator -= animation_speed;
            current_frame = (1 + current_frame) % total_frames;
        }
        animation_iterator++;
    }
};
