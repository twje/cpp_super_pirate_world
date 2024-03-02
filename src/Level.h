#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Interfaces.h"
#include "LevelMap.h"
#include "GameData.h"
#include "Player.h"
#include "Debug.h"

//------------------------------------------------------------------------------
class Level
{
public:
    Level(LevelMap& levelMap, GameData& gameData, IGame& gameCallbacks)
        : mLevelMap(levelMap)
        , mGameData(gameData)    
        , mGameCallbacks(gameCallbacks)
    { 
        mLevelMap.SetDrawObjectLayers(false);

        // Temporary until player is loaded
        mCameraPosition = sf::Vector2f();
    }

    bool HandleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::D)
        {
            mLevelMap.ToggleDrawObjectLayers();
        }

        return true;
    }

    bool Update(const sf::Time& timeslice)
    {
        sf::Vector2f direction;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            direction.x = 1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            direction.x = -1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            direction.y = -1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            direction.y = 1.0f;
        }

        mCameraPosition += direction * timeslice.asSeconds() * 1000.0f;

        return true;
    }

    bool Draw(sf::RenderWindow& window)
    {
        mLevelMap.Draw(window);

        std::string foo = "A";
        if (mLevelMap.GetDrawObjectLayers())
        {
            foo = "B";
        }
        DrawText(window, FontId::DEBUG_FONT, foo, sf::Vector2f());

        return true;
    }

    const sf::Vector2f& GetCameraPosition() { return mCameraPosition; }

private:
    LevelMap& mLevelMap;
    GameData& mGameData;
    IGame& mGameCallbacks;
    sf::Vector2f mCameraPosition;
};