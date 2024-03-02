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
                GameObject* sprite = AddSpriteObject(object);
                mAllSprites.AddGameObject(sprite);                
            }
            // Animated
            else
            {
                std::string id = object.GetName() == "candle" ? "candle_light" : object.GetName();
                GameObject* sprite = AddAnimationObject(object, mGameAssets.GetTextureVec(id), ANIMATION_SPEED);
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
                GameObject* sprite = AddSpriteObject(object);
                mAllSprites.AddGameObject(sprite);
            }
            // Animated
            else
            {
                if (IsSubString(object.GetName(), "palm"))
                {
                    uint32_t animSpeed = ANIMATION_SPEED + RandomInteger(-1, 1);
                    GameObject* sprite = AddAnimationObject(object, 
                                                            mGameAssets.GetTextureDirMap("palms").at(object.GetName()), 
                                                            animSpeed);
                    mAllSprites.AddGameObject(sprite);
                }
                else
                {
                    GameObject* sprite = AddAnimationObject(object, 
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
            // import_sub_folders
        }
    }

    void SetupWater()
    {

    }

    GameObject* AddSpriteObject(const TiledMapObject& object)
    {
        const sf::Texture* texture = mLevelMap.GetTexture(object.GetGid());
        return mGameObjectManager.CreateGameObject<Sprite>(*texture, object.GetPosition());
    }

    GameObject* AddAnimationObject(const TiledMapObject& object, TextureVector& animFrames, uint32_t animSpeed)
    {
        return mGameObjectManager.CreateGameObject<AnimatedSprite>(object.GetPosition(),
                                                                   object.GetScale(),
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