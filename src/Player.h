#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>

// Game
#include "Settings.h"

// Core
#include "Core/GameObject.h"

//------------------------------------------------------------------------------
class Player : public GameObject
{
public:
    Player(const sf::Vector2f& position)
        : mSpeed(200.0f)
    {
        SetPosition(position);
        mPlaceholderSprite.setSize({ 48.0f, 56.0f });
        mPlaceholderSprite.setFillColor(sf::Color::Red);
    }

    virtual FloatRect GetGlobalBounds() const
    {
        return GetTransform().transformRect(mPlaceholderSprite.getLocalBounds());
    }

    virtual void Update(const sf::Time& timeslice)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            mDirection.x = 1.0f;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            mDirection.x = -1.0f;
        }
        else
        {
            mDirection.x = 0.0f;
        }

        sf::Vector2f delta = mDirection * mSpeed * timeslice.asSeconds();
        Move(delta);
    };

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mPlaceholderSprite, statesCopy);
    }

private:
    sf::RectangleShape mPlaceholderSprite;
    sf::Vector2f mDirection;
    float mSpeed;
};