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
    Level(LevelMap& levelMap, GameData& gameData, IGame& gameCallbacks, sf::View& gameView, sf::View& hudView)
        : mLevelMap(levelMap)
        , mGameData(gameData)    
        , mGameCallbacks(gameCallbacks)
        , mGameView(gameView)
        , mHudView(hudView)
    { 
        mLevelMap.SetDrawObjectLayers(false);

        // Objects
        for(const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {            
            if (object.GetName() == "player")
            {
                std::cout << "Found player" << std::endl;
            }
        }

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
        mGameView.setCenter(mCameraPosition);

        return true;
    }

    bool Draw(sf::RenderWindow& window)
    {
        DrawGame(window);
        DrawHUD(window);
        return true;
    }    

private:
    void DrawGame(sf::RenderWindow& window)
    {
        window.setView(mGameView);

        mLevelMap.Draw(window);
    }

    void DrawHUD(sf::RenderWindow& window)
    {
        window.setView(mHudView);

        std::string objectRenderStatus = "Objects Rendered by Level";
        if (mLevelMap.GetDrawObjectLayers())
        {
            objectRenderStatus = "Objects Rendered by TiledMap";
        }

        DrawText(window, FontId::DEBUG_FONT, objectRenderStatus, sf::Vector2f(10.0f, 10.f));
    }

    LevelMap& mLevelMap;
    GameData& mGameData;
    IGame& mGameCallbacks;
    sf::View& mGameView;
    sf::View& mHudView;


    sf::Vector2f mCameraPosition;
};