#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Interfaces.h"
#include "LevelMap.h"
#include "GameData.h"
#include "Player.h"
#include "Debug.h"

// Core
#include "Core/GameObjectManager.h"

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
        , mPlayer(nullptr)
    { 
        mLevelMap.SetDrawObjectLayers(false);

        // Objects
        for(const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {            
            if (object.GetName() == "player")
            {
                mPlayer = GameObjectManager::Instance().CreateGameObject<Player>(sf::Vector2f());
                mAllSprites.AddGameObject(mPlayer);
            }
        }        
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
        for (GameObject* object : mAllSprites)
        {
            object->Update(timeslice);
        }
                
        mGameView.setCenter(mPlayer->GetCameraCenter());

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

        for (GameObject* object : mAllSprites)
        {
            window.draw(*object);
        }
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
    Player* mPlayer;
    Group mAllSprites;    
};