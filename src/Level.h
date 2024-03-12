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
#include "Core/DrawUtils.h"

//------------------------------------------------------------------------------
class Level : public ILevel
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
        Setup();
    }

    bool HandleEvent(const sf::Event& event)
    {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::A)
        {
            mLevelMap.ToggleDrawObjectLayersEnabled();
        }
        
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::D)
        {
            mGameCallbacks.SwitchLevel();
        }

        return true;
    }

    bool Update(const sf::Time& timeslice)
    {
        for (GameObject* object : mAllSprites)
        {
            object->Update(timeslice);
        }
        mPlayer->Update(timeslice);
         
        mGameView.setCenter(mPlayer->GetCameraCenter());

        return true;
    }

    bool Draw(sf::RenderWindow& window)
    {
        DrawGame(window);
        DrawHUD(window);
        return true;
    }    

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

        for (GameObject* object : mAllSprites)
        {
            //DrawRect<float>(window, object->GetGlobalBounds(), sf::Color::Red);
            //DrawRect<float>(window, object->GetHitbox(), sf::Color::Green);
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

private:
#pragma region SetupObjects
    void Setup()
    {
        CreatePlayer();
        CreateTileObjects();
        CreateBackgroundDetail();
        CreateStaticObjects();
        CreateMovingObjects();
        CreateEnemies();
        CreateItems();
        CreateWater();
    }

    void CreatePlayer()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {
            if (object.GetName() == "player")
            {
                GameObjectManager& gameObjectManager = GameObjectManager::Instance();
                mPlayer = gameObjectManager.CreateGameObject<Player>(object.GetPosition(),
                                                                     mGameAssets.GetTextureDirMap("player"),
                                                                     mCollisionSprites,
                                                                     mSemiCollisionSprites,
                                                                     mGameData);                
                mDrawGroups[mPlayer->GetDepth()].AddGameObject(mPlayer);
            }
        }
    }

    void CreateTileObjects()
    {        
        sf::Vector2f tileSize = mLevelMap.GetTileSize();
        for (const auto& [coord, tile] : mLevelMap.GetTileDataByLayerName("Terrain"))
        {
            int32_t coordX = std::get<0>(coord);
            int32_t coordY = std::get<1>(coord);            
            
            Sprite* sprite = CreateGameObject<Sprite>(mLevelMap.GetTexture(tile->GetGid()),
                                                      tile->GetTextureRegion(),
                                                      sf::Vector2f(coordX * tileSize.x, coordY * tileSize.y),
                                                      0);  // TODO: fix depth
            mAllSprites.AddGameObject(sprite);
            mCollisionSprites.AddGameObject(sprite);
        }
    }

    void CreateBackgroundDetail()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("BG details"))
        {
            // Static
            if (object.GetName() == "static")
            {
                GameObject* sprite = CreateGameObject<Sprite>(mLevelMap.GetTexture(object.GetGid()), 
                                                              object.GetPosition(), 
                                                              DEPTHS.at("bg tiles"));
                AddToCommonGroups(sprite);
            }
            // Animated
            else
            {
                std::string id = object.GetName() == "candle" ? "candle_light" : object.GetName();
                GameObject* sprite = CreateGameObject<AnimatedSpriteImpl>(object.GetPosition(),
                                                                          object.GetScale(),
                                                                          mGameAssets.GetTextureVec(id), 
                                                                          ANIMATION_SPEED,
                                                                          DEPTHS.at("bg tiles"));
                AddToCommonGroups(sprite);
            }
        }
    }

    void CreateStaticObjects()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Objects"))
        {
            if (object.GetName() == "player")
            {
                continue;
            }
            
            // Static
            if (object.GetName() == "barrel" || object.GetName() == "crate")
            {
                GameObject* sprite = CreateGameObject<Sprite>(mLevelMap.GetTexture(object.GetGid()), 
                                                              object.GetPosition(), 
                                                              DEPTHS.at("main"));
                AddToCommonGroups(sprite);
                mCollisionSprites.AddGameObject(sprite);
            }
            // Animated
            else
            {
                uint32_t depth = IsSubString(object.GetName(), "bg") ? DEPTHS.at("bg details") : DEPTHS.at("main");
                
                if (IsSubString(object.GetName(), "palm"))
                {
                    GameObject* sprite = CreateGameObject<AnimatedSpriteImpl>(object.GetPosition(),
                                                                              object.GetScale(),
                                                                              mGameAssets.GetTextureDirMap("palms").at(object.GetName()),
                                                                              ANIMATION_SPEED + RandomInteger(-1, 1),
                                                                              depth);
                    AddToCommonGroups(sprite);

                    if (object.GetName() == "palm_small" || object.GetName() == "palm_large")
                    {
                        mSemiCollisionSprites.AddGameObject(sprite);
                    }
                }
                else
                {
                    GameObject* sprite = CreateGameObject<AnimatedSpriteImpl>(object.GetPosition(),
                                                                              object.GetScale(),
                                                                              mGameAssets.GetTextureVec(object.GetName()),
                                                                              ANIMATION_SPEED,
                                                                              depth);
                    AddToCommonGroups(sprite);

                    if (object.GetName() == "saw" || object.GetName() == "floor_spike")
                    {
                        mDemageSprites.AddGameObject(sprite);
                    }
                }
            }
        
            if (object.GetName() == "flag")
            {
                // implement
            }
        }
    }

    void CreateMovingObjects()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Moving Objects"))
        {
            if (object.GetName() == "spike")
            {
                float radius = static_cast<float>(object.GetPropertyValue<int32_t>("radius"));
                float speed = static_cast<float>(object.GetPropertyValue<int32_t>("speed"));
                float startAngle = static_cast<float>(object.GetPropertyValue<int32_t>("start_angle"));
                float endAngle = static_cast<float>(object.GetPropertyValue<int32_t>("end_angle"));
                
                GameObject* sprite = CreateGameObject<Spike>(mGameAssets.GetTexture("spike"),
                                                             object.GetPosition(),
                                                             radius,
                                                             speed,
                                                             startAngle,
                                                             endAngle,
                                                             DEPTHS.at("main"));
                AddToCommonGroups(sprite);
                mDemageSprites.AddGameObject(sprite);
                
                for (float newRadius = 0.0f; newRadius < radius; newRadius += 20.0f)
                {
                    GameObject* sprite = CreateGameObject<Spike>(mGameAssets.GetTexture("spike_chain"),
                                                                 object.GetPosition(),
                                                                 newRadius,
                                                                 speed,
                                                                 startAngle,
                                                                 endAngle,
                                                                 DEPTHS.at("bg details"));
                    AddToCommonGroups(sprite);
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
                GameObject* sprite = CreateGameObject<MovingSprite>(startPos,
                                                                    endPos,
                                                                    mIsVertMovement,
                                                                    speed,
                                                                    object.GetScale(),
                                                                    mGameAssets.GetTextureVec(object.GetName()),
                                                                    ANIMATION_SPEED,
                                                                    object.GetPropertyValue<bool>("flip"));
                AddToCommonGroups(sprite);
                
                if (object.GetPropertyValue<bool>("platform"))
                {
                    mSemiCollisionSprites.AddGameObject(sprite);
                }
                else
                {
                    mDemageSprites.AddGameObject(sprite);
                }

                if (object.GetName() == "saw")
                {
                    const sf::Texture& texture = mGameAssets.GetTexture("saw_chain");
                    if (mIsVertMovement)
                    {                        
                        float x = startPos.x - texture.getSize().x / 2.0f;
                        for (float y = startPos.y; y < endPos.y; y += 20.0f)
                        {
                            GameObject* sprite = CreateGameObject<Sprite>(texture, sf::Vector2f(x, y), DEPTHS.at("bg details"));                            
                            AddToCommonGroups(sprite);
                        }
                    }
                    else
                    {
                        float y = startPos.y - texture.getSize().y / 2.0f;
                        for (float x = startPos.x; x < endPos.x; x += 20.0f)
                        {
                            GameObject* sprite = CreateGameObject<Sprite>(texture, sf::Vector2f(x, y), DEPTHS.at("bg details"));
                            AddToCommonGroups(sprite);
                        }
                    }
                }
            }
        }
    }

    void CreateEnemies()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Enemies"))
        {
            if (object.GetName() == "tooth")
            {                
                GameObject* sprite = CreateGameObject<Tooth>(object.GetPosition(),
                                                             object.GetScale(),
                                                             mGameAssets.GetTextureVec(object.GetName()),
                                                             ANIMATION_SPEED,
                                                             mCollisionSprites);                                                                              
                AddToCommonGroups(sprite);
                mDemageSprites.AddGameObject(sprite);
                mToothSprites.AddGameObject(sprite);
            }
            else if (object.GetName() == "shell")
            {
                GameObject* sprite = CreateGameObject<Shell>(object.GetPosition(),
                                                             object.GetPropertyValue<bool>("reverse"),
                                                             mGameAssets.GetTextureDirMap(object.GetName()),
                                                             ANIMATION_SPEED,
                                                             *mPlayer,
                                                             *this);
                AddToCommonGroups(sprite);
                mCollisionSprites.AddGameObject(sprite);
            }
        }
    }

    void CreateItems()
    {
        for (const TiledMapObject& object : mLevelMap.GetObjectsByLayerName("Items"))
        {                    
            GameObject* sprite = CreateGameObject<Item>(object.GetName(),
                                                        object.GetPosition() + mLevelMap.GetTileSize() * 0.5f,
                                                        object.GetScale(),
                                                        mGameAssets.GetTextureDirMap("items").at(object.GetName()),
                                                        ANIMATION_SPEED,
                                                        mGameData);
            AddToCommonGroups(sprite);
            mItemSprites.AddGameObject(sprite);
        }
    }

    void CreateWater()
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
                        GameObject* sprite = CreateGameObject<AnimatedSpriteImpl>(sf::Vector2f(x, y),
                                                                                  object.GetScale(),
                                                                                  mGameAssets.GetTextureVec("water_top"),
                                                                                  ANIMATION_SPEED,
                                                                                  DEPTHS.at("water"));
                        AddToCommonGroups(sprite);
                    }
                    else
                    {
                        GameObject* sprite = CreateGameObject<Sprite>(mGameAssets.GetTexture("water_body"), 
                                                                     sf::Vector2f(x, y), 
                                                                     DEPTHS.at("water"));
                        AddToCommonGroups(sprite);
                    }
                }
            }
        }
    }

    virtual void CreatePearl(const sf::Vector2f& position, float direction) override
    {
        GameObject* sprite = CreateGameObject<Pearl>(mGameAssets.GetTexture("pearl"), position, direction, 150);
        AddToCommonGroups(sprite);
        mDemageSprites.AddGameObject(sprite);
        mPearlSprites.AddGameObject(sprite);
    }

    template<typename T, typename... Args>
    T* CreateGameObject(Args&&... args)
    {
        return GameObjectManager::Instance().CreateGameObject<T>(std::forward<Args>(args)...);
    }

    void AddToCommonGroups(GameObject* sprite)
    {
        mAllSprites.AddGameObject(sprite);
        mDrawGroups[sprite->GetDepth()].AddGameObject(sprite);
    }

#pragma endregion
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
    Group mCollisionSprites;
    Group mSemiCollisionSprites;
    Group mDemageSprites;
    Group mToothSprites;
    Group mPearlSprites;
    Group mItemSprites;    
};