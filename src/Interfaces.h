#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>

//------------------------------------------------------------------------------
class IGame
{
public:
    virtual void SwitchLevel() = 0;
};

//------------------------------------------------------------------------------
class ILevel
{
public:
    virtual void CreatePearl(const sf::Vector2f& position, float direction) = 0;
};