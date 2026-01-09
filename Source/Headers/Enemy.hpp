#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Global.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"


class Enemy : public std::enable_shared_from_this<Enemy> {
protected:
    float x;
    float y;
    float horizontal_speed = 0;
    float vertical_speed = 0;
    bool dead = false;

public:
    constexpr Enemy(float x, float y) : x(x), y(y) {}

    virtual ~Enemy() = default;

    virtual bool get_dead(bool deletion) const {
        return dead;
    }

    virtual void die(uint8_t death_type) {
        dead = true;
    }

    //Apparently, we can set the function declaration to 0 and that's called a pure virtual function. Again, C++ is weird.
    virtual void draw(unsigned view_x, sf::RenderWindow& window) = 0;

    virtual void update(unsigned view_x, const std::vector<std::shared_ptr<Enemy>>& enemies,
                        const MapManager& map_manager, Mario& mario) = 0;

    sf::FloatRect get_hit_box() const {
        return {x, y, CELL_SIZE, CELL_SIZE};
    }
};
