#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"
#include "GameData.h"
#include "Interfaces.h"
#include "LevelMap.h"
#include "Level.h"
#include "Player.h"

// Third party
#include <SFML/Graphics.hpp>

// Core
#include "Core/CustomExceptions.h"
#include "Core/GameObjectManager.h"
#include "Core/LayerStack.h"
#include "Core/ResourceManager.h"

//------------------------------------------------------------------------------
class Game : public Layer, public IGame
{
public:
    Game(LayerStack& layerStack, const sf::Vector2u& windowSize)
        : Layer(layerStack)        
        , mPosition(sf::Vector2f(windowSize) / 2.0f)
    {        
        mGameView.setSize(sf::Vector2f(windowSize));
        mGameView.setCenter(sf::Vector2f(windowSize) / 2.0f);        

        mLevelMaps.emplace(0, "data/levels/omni.json");
        mLevelMaps.emplace(1, "data/levels/1.json");
        mLevelMaps.emplace(2, "data/levels/2.json");
        mLevelMaps.emplace(3, "data/levels/3.json");
        mLevelMaps.emplace(4, "data/levels/4.json");
        mLevelMaps.emplace(5, "data/levels/5.json");

        mCurrentLevel = std::make_unique<Level>(mLevelMaps.at(mGameData.GetCurrentLevel()), mGameData, *this);
        LoadGlobalAssets();
    }    

    virtual bool HandleEvent(const sf::Event& event) 
    { 
        return mCurrentLevel->HandleEvent(event);
    };

    virtual void Resize(const sf::Vector2f& size) override
    {
        mGameView.setSize(size);
    }

    virtual bool Update(const sf::Time& timeslice) override
    {
        bool drawNextLayer = mCurrentLevel->Update(timeslice);

        for (GameObject* obj : mAllSprites)
        {
            obj->Update(timeslice);
        }

        mGameView.setCenter(mCurrentLevel->GetCameraPosition());

        return drawNextLayer;
    }

    virtual bool Draw(sf::RenderWindow& window) override
    {
        window.setView(mGameView);        

        bool updateNextLayer = mCurrentLevel->Draw(window);

        for (GameObject* obj : mAllSprites)
        {
            window.draw(*obj);
        }

        return updateNextLayer;
    }

    void UnloadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().ReleaseResource(filepath);
        }
    }

private:
    void LoadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().RequireResource(filepath);
        }        
    }

    virtual void SwitchLevel() override
    {
        throw NotImplementedException();
    }

    Group mAllSprites;
    sf::View mGameView;
    
    std::unordered_map<uint32_t, LevelMap> mLevelMaps;
    sf::Vector2f mPosition;
    GameData mGameData;
    std::unique_ptr<Level> mCurrentLevel;
};

//------------------------------------------------------------------------------
int main()
{    
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "SFML Template Project");
    window.setVerticalSyncEnabled(true);

    sf::Clock clock;
    const sf::Time timePerFrame = sf::seconds(1.0f / 60.0f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    LayerStack layerStack;
    layerStack.PushLayer(std::make_unique<Game>(layerStack, window.getSize()));    
    Game* game = static_cast<Game*>(layerStack.GetTopLayer());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                game->UnloadGlobalAssets();
                window.close();
            }
            else
            {
                layerStack.HandleEvent(event);
            }
        }

        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate >= timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            layerStack.Update(timePerFrame);
            GameObjectManager::Instance().SyncGameObjectChanges();

            window.clear();
            layerStack.Draw(window);
            window.display();
        }
    }

    return 0;
}