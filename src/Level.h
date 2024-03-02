#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Interfaces.h"
#include "LevelMap.h"
#include "GameData.h"
#include "GameAssets.h"
#include "Player.h"
#include "Debug.h"

// Core
#include "Core/GameObjectManager.h"
#include "Core/StringUtils.h"
#include "Core/RandomUtils.h"
#include "Core/Animate.h"

//------------------------------------------------------------------------------
class Sprite : public GameObject
{
public:
    Sprite(const sf::Texture& texture, const sf::Vector2f& position)
        : mSprite(texture)
    {
        mSprite.setPosition(position);
    }

    virtual FloatRect GetGlobalBounds() const 
    {  
        return GetTransform().transformRect(mSprite.getLocalBounds());
    }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

private:
    sf::Sprite mSprite;
};

//------------------------------------------------------------------------------
class Item : public Sprite
{
public:
    Item(const sf::Texture& texture, const sf::Vector2f& position)
        : Sprite(texture, position)
    { }
};

//------------------------------------------------------------------------------
class AnimatedSprite : public GameObject
{
public:
    AnimatedSprite(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
        : mAnimation(animSpeed)
        , mSprite(*animFrames[0])
    {     
        SetScale(scale);
        SetPosition(position);

        auto animFramesCopy = std::make_unique<std::vector<sf::Texture*>>();
        for (auto& texturePtr : animFrames)
        {
            animFramesCopy->push_back(&(*texturePtr));
        }               
        mAnimation.AddSequence({ "current", std::move(animFramesCopy) });
        mAnimation.SetSequence("current");
    }

    virtual FloatRect GetGlobalBounds() const
    {        
        return GetTransform().transformRect(mSprite.getLocalBounds());
    }

    virtual void Update(const sf::Time& timeslice)
    {
        mAnimation.Update(timeslice);
        mSprite.setTexture(mAnimation.GetTexture(), true);
    }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

private:
    Animation mAnimation;    
    sf::Sprite mSprite;
};

//------------------------------------------------------------------------------
class Level
{
public:
    Level(LevelMap& levelMap, GameData& gameData, GameAssets& gameAssets, IGame& gameCallbacks, sf::View& gameView, sf::View& hudView)
        : mLevelMap(levelMap)
        , mGameData(gameData) 
        , mGameAssets(gameAssets)
        , mGameCallbacks(gameCallbacks)
        , mGameView(gameView)
        , mHudView(hudView)
        , mGameObjectManager(GameObjectManager::Instance())
        , mPlayer(nullptr)
    { 
        mLevelMap.SetDrawObjectLayers(false);
        Setup();
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
    void Setup()
    {
        SetupBackgroundDetails();
        SetupObjects();
        SetupMovingObjects();
        SetupEnemies();
        SetupItems();
        SetupWater();
    }

    void SetupBackgroundDetails()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("BG details"))
        {
            // Static
            if (object.GetName() == "static")
            {
                GameObject* sprite = AddSpriteObject(mLevelMap.GetTexture(object.GetGid()), object.GetPosition());
                mAllSprites.AddGameObject(sprite);
            }
            // Animated
            else
            {
                std::string id = object.GetName() == "candle" ? "candle_light" : object.GetName();
                GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                        object.GetScale(),
                                                        mGameAssets.GetTextureVec(id), 
                                                        ANIMATION_SPEED);
                mAllSprites.AddGameObject(sprite);
            }
        }
    }

    void SetupObjects()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {
            if (object.GetName() == "player")
            {
                mPlayer = mGameObjectManager.CreateGameObject<Player>(sf::Vector2f());
                mAllSprites.AddGameObject(mPlayer);
            }
            // Static
            else if (object.GetName() == "barrel" || object.GetName() == "crate")
            {
                GameObject* sprite = AddSpriteObject(mLevelMap.GetTexture(object.GetGid()), object.GetPosition());
                mAllSprites.AddGameObject(sprite);
            }
            // Animated
            else
            {
                if (IsSubString(object.GetName(), "palm"))
                {
                    uint32_t animSpeed = ANIMATION_SPEED + RandomInteger(-1, 1);
                    GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                            object.GetScale(),
                                                            mGameAssets.GetTextureDirMap("palms").at(object.GetName()), 
                                                            animSpeed);
                    mAllSprites.AddGameObject(sprite);
                }
                else
                {                   
                        GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                                object.GetScale(),
                                                                mGameAssets.GetTextureVec(object.GetName()),
                                                                ANIMATION_SPEED);
                        mAllSprites.AddGameObject(sprite);
                
                }
            }
        }
    }

    void SetupMovingObjects()
    {

    }        

    void SetupEnemies()
    {

    }

    void SetupItems()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Items"))
        {            
            GameObject* sprite = AddItemObject(mLevelMap.GetTexture(object.GetGid()), object.GetPosition());
            mAllSprites.AddGameObject(sprite);
        }
    }

    void SetupWater()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Water"))
        {
            sf::Vector2f tilesize = mLevelMap.GetTileSize();
            uint32_t rows = static_cast<uint32_t>(std::round(object.GetSize().y / tilesize.y));
            uint32_t cols = static_cast<uint32_t>(std::round(object.GetSize().x / tilesize.x));

            for (uint32_t row = 0; row < rows; row++)
            {
                for (uint32_t col = 0; col < cols; col++)
                {
                    float x = static_cast<float>(object.GetPosition().x + col * tilesize.x);
                    float y = static_cast<float>(object.GetPosition().y + row * tilesize.y);
                    
                    if (row == 0)
                    {
                        GameObject* sprite = AddAnimationObject({ x, y },
                                                                object.GetScale(),
                                                                mGameAssets.GetTextureVec("water_top"),
                                                                ANIMATION_SPEED);
                        mAllSprites.AddGameObject(sprite);
                    }
                    else
                    {
                        GameObject* sprite = AddSpriteObject(mGameAssets.GetTexture("water_body"), { x, y });
                        mAllSprites.AddGameObject(sprite);
                    }
                }
            }
        }
    }

    GameObject* AddItemObject(const sf::Texture& texture, const sf::Vector2f& position)
    { 
        return mGameObjectManager.CreateGameObject<Item>(texture, position);
    }

    GameObject* AddSpriteObject(const sf::Texture& texture, const sf::Vector2f& position)
    {
        return mGameObjectManager.CreateGameObject<Sprite>(texture, position);
    }

    GameObject* AddAnimationObject(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
    {
        return mGameObjectManager.CreateGameObject<AnimatedSprite>(position,
                                                                   scale,
                                                                   animFrames,
                                                                   animSpeed);
    }

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
    GameAssets& mGameAssets;
    IGame& mGameCallbacks;
    sf::View& mGameView;
    sf::View& mHudView;
    GameObjectManager& mGameObjectManager;
    Player* mPlayer;
    Group mAllSprites;
};