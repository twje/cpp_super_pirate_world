// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>

// Game
#include "Settings.h"
#include "Player.h"

// Core
#include "Core/GameObjectManager.h"
#include "Core/LayerStack.h"

//------------------------------------------------------------------------------
class Game : public Layer
{
public:
    Game(LayerStack& layerStack, const sf::Vector2u& windowSize)
        : Layer(layerStack)
    {
        mGameView.setSize(sf::Vector2f(windowSize));
        mGameView.setCenter(sf::Vector2f(windowSize) / 2.0f);

        GameObjectManager& manager = GameObjectManager::Instance();       
        Player* player = manager.CreateGameObject<Player>(sf::Vector2f());
        mAllSprites.AddGameObject(player);
    }

    virtual void Resize(const sf::Vector2f& size) override
    {
        mGameView.setSize(size);
    }

    virtual bool Update(const sf::Time& timeslice) override
    {
        for (GameObject* obj : mAllSprites)
        {
            obj->Update(timeslice);
        }
        return true;
    }

    virtual bool Draw(sf::RenderWindow& window) override
    {
        window.setView(mGameView);

        for (GameObject* obj : mAllSprites)
        {
            window.draw(*obj);
        }
        return true;
    }

private:
    Group mAllSprites;
    sf::View mGameView;
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

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
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