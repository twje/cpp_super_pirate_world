#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Interfaces.h"
#include "LevelMap.h"
#include "GameData.h"
#include "GameAssets.h"
#include "Debug.h"
#include "Player.h"
#include "Sprites.h"

// Core
#include "Core/GameObjectManager.h"
#include "Core/StringUtils.h"
#include "Core/RandomUtils.h"

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
        , mPlayer(nullptr)
    {
        mLevelMap.SetDrawObjectLayersEnabled(false);
        mLevelMap.AddDrawabeLayer("BG", DEPTHS.at("bg tiles"));
        mLevelMap.AddDrawabeLayer("FG", DEPTHS.at("bg tiles"));
        mLevelMap.AddDrawabeLayer("Terrain", DEPTHS.at("main"));
        mLevelMap.AddDrawabeLayer("Platforms", DEPTHS.at("main"));

        Setup();
    }

    bool HandleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::D)
        {
            mLevelMap.ToggleDrawObjectLayersEnabled();
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

#pragma region SetupObjects
    void SetupBackgroundDetails()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("BG details"))
        {
            // Static
            if (object.GetName() == "static")
            {
                GameObject* sprite = AddSpriteObject(mLevelMap.GetTexture(object.GetGid()), 
                                                     object.GetPosition(), 
                                                     DEPTHS.at("bg tiles"));
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
            }
            // Animated
            else
            {
                std::string id = object.GetName() == "candle" ? "candle_light" : object.GetName();
                GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                        object.GetScale(),
                                                        mGameAssets.GetTextureVec(id), 
                                                        ANIMATION_SPEED,
                                                        DEPTHS.at("bg tiles"));
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
            }
        }
    }

    void SetupObjects()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {
            if (object.GetName() == "player")
            {
                GameObjectManager& gameObjectManager = GameObjectManager::Instance();
                mPlayer = gameObjectManager.CreateGameObject<Player>(sf::Vector2f());
                mAllSprites.AddGameObject(mPlayer);                
                mDrawGroups[mPlayer->GetDepth()].AddGameObject(mPlayer);
            }
            // Static
            else if (object.GetName() == "barrel" || object.GetName() == "crate")
            {
                GameObject* sprite = AddSpriteObject(mLevelMap.GetTexture(object.GetGid()), object.GetPosition(), DEPTHS.at("main"));
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
            }
            // Animated
            else
            {
                uint32_t depth = IsSubString(object.GetName(), "bg") ? DEPTHS.at("bg details") : DEPTHS.at("main");
                
                if (IsSubString(object.GetName(), "palm"))
                {
                    GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                            object.GetScale(),
                                                            mGameAssets.GetTextureDirMap("palms").at(object.GetName()),
                                                            ANIMATION_SPEED + RandomInteger(-1, 1),
                                                            depth);
                    mAllSprites.AddGameObject(sprite);
                    mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                }
                else
                {
                    GameObject* sprite = AddAnimationObject(object.GetPosition(),
                                                            object.GetScale(),
                                                            mGameAssets.GetTextureVec(object.GetName()),
                                                            ANIMATION_SPEED,
                                                            depth);
                    mAllSprites.AddGameObject(sprite);
                    mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                }
            }
        
            if (object.GetName() == "flag")
            {
                // implement
            }
        }
    }

    void SetupMovingObjects()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Moving Objects"))
        {
            if (object.GetName() == "spike")
            {
                int32_t radius = object.GetPropertyValue<int32_t>("radius");
                int32_t speed = object.GetPropertyValue<int32_t>("speed");
                int32_t startAngle = object.GetPropertyValue<int32_t>("start_angle");
                int32_t endAngle = object.GetPropertyValue<int32_t>("end_angle");
                
                GameObject* sprite = AddSpikeObject(mGameAssets.GetTexture("spike"),
                                                    object.GetPosition(),
                                                    radius,
                                                    speed,
                                                    startAngle,
                                                    endAngle,
                                                    DEPTHS.at("main"));
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                
                for (int32_t newRadius = 0; newRadius < radius; newRadius += 20)
                {
                    GameObject* sprite = AddSpikeObject(mGameAssets.GetTexture("spike_chain"),
                                                        object.GetPosition(),
                                                        newRadius,
                                                        speed,
                                                        startAngle,
                                                        endAngle,
                                                        DEPTHS.at("bg details"));
                    mAllSprites.AddGameObject(sprite);
                    mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                }
            }
            else
            {

                bool mIsVertMovement = false;
                sf::Vector2f startPos(object.GetPosition().x, object.GetPosition().y);
                sf::Vector2f endPos(object.GetPosition().x + object.GetSize().x, object.GetPosition().y);
                
                if (object.GetSize().y > object.GetSize().x)  
                {
                    mIsVertMovement = true;
                    startPos = sf::Vector2f(object.GetPosition().x, object.GetPosition().y);
                    endPos = sf::Vector2f(object.GetPosition().x, object.GetPosition().y + object.GetSize().y);
                }

                int32_t speed = object.GetPropertyValue<int32_t>("speed");
                GameObject* sprite = AddMovementSpriteObject(startPos,
                                                             endPos,
                                                             mIsVertMovement,
                                                             speed,
                                                             object.GetScale(),
                                                             mGameAssets.GetTextureVec(object.GetName()),
                                                             ANIMATION_SPEED);
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);

                if (object.GetName() == "saw")
                {
                    const sf::Texture& texture = mGameAssets.GetTexture("saw_chain");
                    if (mIsVertMovement)
                    {                        
                        float x = startPos.x - texture.getSize().x / 2.0f;
                        for (float y = startPos.y; y < endPos.y; y += 20.0f)
                        {
                            GameObject* sprite = AddSpriteObject(texture, { x, y }, DEPTHS.at("bg details"));
                            mAllSprites.AddGameObject(sprite);
                            mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                        }
                    }
                    else
                    {
                        float y = startPos.y - texture.getSize().y / 2.0f;
                        for (float x = startPos.x; x < endPos.x; x += 20.0f)
                        {
                            GameObject* sprite = AddSpriteObject(texture, { x, y }, DEPTHS.at("bg details"));
                            mAllSprites.AddGameObject(sprite);
                            mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                        }
                    }
                }
            }
        }
    }

    void SetupEnemies()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Enemies"))
        {
            if (object.GetName() == "tooth")
            {
                GameObject* sprite = AddToothObject(object.GetPosition(),
                                                    object.GetScale(),
                                                    mGameAssets.GetTextureVec(object.GetName()),
                                                    ANIMATION_SPEED);
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
            }
            else if (object.GetName() == "shell")
            {
                GameObject* sprite = AddShellObject(object.GetPosition(),
                                                    object.GetScale(),
                                                    mGameAssets.GetTextureDirMap(object.GetName()),
                                                    ANIMATION_SPEED);
                mAllSprites.AddGameObject(sprite);
                mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
            }
        }
    }

    void SetupItems()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Items"))
        {                    
            GameObject* sprite = AddItemObject(object.GetPosition() + mLevelMap.GetTileSize() * 0.5f,
                                               object.GetScale(),
                                               mGameAssets.GetTextureDirMap("items").at(object.GetName()),
                                               ANIMATION_SPEED);
            mAllSprites.AddGameObject(sprite);
            mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
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
                                                                ANIMATION_SPEED,
                                                                DEPTHS.at("water"));
                        mAllSprites.AddGameObject(sprite);
                        mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                    }
                    else
                    {
                        GameObject* sprite = AddSpriteObject(mGameAssets.GetTexture("water_body"), { x, y }, DEPTHS.at("water"));
                        mAllSprites.AddGameObject(sprite);
                        mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region ObjectFactories
    GameObject* AddMovementSpriteObject(const sf::Vector2f& startPos, const sf::Vector2f& endPos, bool isVertMovement, int32_t speed,
        const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
    {
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<MovingSprite>(startPos, endPos, isVertMovement, speed, scale, animFrames, animSpeed);
    }

    GameObject* AddSpikeObject(const sf::Texture& texture, const sf::Vector2f& position, int32_t radius, int32_t speed, 
                               int32_t startAngle, int32_t endAngle, uint32_t depth)
    {
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<Spike>(texture, position, radius, speed, startAngle, endAngle, depth);
    }

    GameObject* AddItemObject(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
    { 
        // progress
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<Item>(position, scale, animFrames, animSpeed);
    }

    GameObject* AddSpriteObject(const sf::Texture& texture, const sf::Vector2f& position, uint32_t depth)
    {
        // check
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<Sprite>(texture, position, depth);     
    }

    GameObject* AddAnimationObject(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed, 
                                   uint32_t depth)
    {
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<AnimatedSprite>(position,
                                                                  scale,
                                                                  animFrames,
                                                                  animSpeed,
                                                                  depth);
    }
    
    GameObject* AddToothObject(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
    {
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<Tooth>(position,
                                                         scale,
                                                         animFrames,
                                                         animSpeed);
    }
    
    GameObject* AddShellObject(const sf::Vector2f& position, const sf::Vector2f& scale, TextureMap& animFrames, uint32_t animSpeed)
    {
        GameObjectManager& gameObjectManager = GameObjectManager::Instance();
        return gameObjectManager.CreateGameObject<Shell>(position,
                                                         scale,
                                                         animFrames,
                                                         animSpeed);
    }
#pragma endregion

    void DrawGame(sf::RenderWindow& window)
    {
        window.setView(mGameView);
        
        for (const uint32_t& depth : GetSortedDepths())
        {
            mLevelMap.Draw(window, depth);

            for (const GameObject* object : mDrawGroups[depth])
            {
                window.draw(*object);
            }
        }
    }

    void DrawHUD(sf::RenderWindow& window)
    {
        window.setView(mHudView);

        std::string objectRenderStatus = "Objects Rendered by Level";
        if (mLevelMap.IsDrawObjectLayersEnabled())
        {
            objectRenderStatus = "Objects Rendered by TiledMap";
        }

        DrawText(window, FontId::DEBUG_FONT, objectRenderStatus, sf::Vector2f(10.0f, 10.f));
    }
    
    LevelMap& mLevelMap;
    GameData& mGameData;
    GameAssets& mGameAssets;
    IGame& mGameCallbacks;    
    Player* mPlayer;

    // Views
    sf::View& mGameView;
    sf::View& mHudView;

    // Groups
    std::unordered_map<uint32_t, Group> mDrawGroups;
    Group mAllSprites;
};