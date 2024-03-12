#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "BaseSprites.h"


// Core
#include "Core/DrawUtils.h"
#include "Core/GameObjectManager.h"

//------------------------------------------------------------------------------
class Player : public AnimatedSprite
{
public:
    Player(const sf::Vector2f& position, TextureMap& animFrames, Group& collisionSprites, Group& semiCollisionSprites, 
           GameData& gameData)
        : AnimatedSprite(position, { 1.0f, 1.0f }, animFrames["idle"], ANIMATION_SPEED, DEPTHS.at("player"))
        , mCollisionSprites(collisionSprites)
        , mSemiCollisionSprites(semiCollisionSprites)
        , mState("idle")
        , mSpeed(200.0f)
        , mGravity(1300.0f)
        , mJumpHeight(900)
        , mIsFacingRight(true)  
        , mIsJumping(false)
        , mIsAttacking(false)
        , mPlatformId(0)
    {
        for (const auto& [sequenceId, frames] : animFrames)
        {
            AddAnimationSequence(sequenceId, frames);
        }
        SetAnimationSequence(mState);

        mSurfaceState = {
            { "floor", false },
            { "left", false },
            { "right", false },
        };
        
        SetOrigin(GetGlobalBounds().GetSize() * 0.5f);
        Move(GetGlobalBounds().GetSize() * 0.5f);
        mHitbox = InflateRect(GetGlobalBounds(), -76, -36);
        mPreviousHitbox = mHitbox;
    }

    virtual FloatRect GetHitbox() const override { return mHitbox; }
    virtual FloatRect GetPreviousHitbox() const override { return mPreviousHitbox; }

    virtual void Update(const sf::Time& timeslice)
    {
        mPreviousHitbox = mHitbox;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            mDirection.x = 1.0f;
            mIsFacingRight = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            mDirection.x = -1.0f;
            mIsFacingRight = false;
        }
        else
        {
            mDirection.x = 0.0f;
        }       

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            mIsJumping = true;            
        }

        // Hort movement
        float deltaX = mDirection.x * mSpeed * timeslice.asSeconds();
        mHitbox.MoveX(deltaX);        
        HortCollision();
        
        // Vert movement
        // TEST

        mDirection.y += mGravity * 0.5f * timeslice.asSeconds();
        // https://stackoverflow.com/questions/60198718/gravity-strength-and-jump-height-somehow-dependant-on-framerate-pygame
        // This step accounts for the distance the object travels while accelerating
        mHitbox.MoveY(mDirection.y * timeslice.asSeconds());
        mDirection.y += mGravity * 0.5f * timeslice.asSeconds(); // TODO: investigate
        
        if (mIsJumping)
        {
            if (mSurfaceState["floor"])
            {
                mDirection.y = -mJumpHeight;
                // Prevent vertical collision response from sticking the player to the ground.
                mHitbox.MoveY(-1);                
            }
            mIsJumping = false;
        }
        
        VertCollision();
        SetPosition(mHitbox.GetCenter());    

        // AFTER
        CheckContact();
        PlatformMove(timeslice);
        SemiCollision();

        UpdateState();
        Animate(timeslice);
    };

    sf::Vector2f GetCameraCenter() { return GetGlobalBounds().GetCenter(); }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        DrawRect<float>(target, CreateFloorCollider(), sf::Color::Green);
        DrawRect<float>(target, CreateLeftWallCollider(), sf::Color::Green);
        DrawRect<float>(target, CreateRightWallCollider(), sf::Color::Green);
        DrawRect<float>(target, GetHitbox(), sf::Color::Red);
        Sprite::draw(target, states);

        if (mPlatformId > 0)
        {
            GameObjectManager& manager = GameObjectManager::Instance();
            if (GameObject* object = manager.GetInstance(mPlatformId))
            {
                DrawRect<float>(target, object->GetHitbox(), sf::Color::Yellow);
            }
        }
    }

private:
    //void Move(const sf::Time& timeslice)
    //{

    //}

    void PlatformMove(const sf::Time& timeslice);

    void CheckContact()
    {
        CheckFloorContact();
        CheckWallContact();
        CheckPlatformContact();        
    }

    void CheckFloorContact()
    {
        mSurfaceState["floor"] = false;
        bool floorContactDetected = false;

        if (mDirection.y >= 0.0f)
        {
            FloatRect floorCollider = CreateFloorCollider();

            for (GameObject* object : mCollisionSprites)
            {
                if (floorContactDetected) { break; }

                FloatRect objectHitbox = object->GetHitbox();
                if (floorCollider.FindIntersection(objectHitbox))
                {
                    mSurfaceState["floor"] = true;
                    floorContactDetected = true;
                }
            }

            for (GameObject* object : mSemiCollisionSprites)
            {
                if (floorContactDetected) { break; }

                FloatRect objectHitbox = object->GetHitbox();
                if (floorCollider.FindIntersection(objectHitbox))
                {
                    mSurfaceState["floor"] = true;
                    floorContactDetected = true;
                }
            }
        }
    }

    void CheckWallContact()
    {
        // Implement
    }

    void CheckPlatformContact()
    {
        mPlatformId = 0;
        FloatRect floorCollider = CreateFloorCollider();        
        bool platformContactDetected = false;

        for (GameObject* object : mCollisionSprites)
        {
            if (platformContactDetected) { break; }

            FloatRect objectHitbox = object->GetHitbox();
            if (floorCollider.FindIntersection(objectHitbox))
            {             
                if (object->GetType() == static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM))
                {
                    mPlatformId = object->GetEntityId();
                    platformContactDetected = true;
                }
            }
        }

        for (GameObject* object : mSemiCollisionSprites)
        {   
            if (platformContactDetected) { break; }

            FloatRect objectHitbox = object->GetHitbox();
            if (floorCollider.FindIntersection(objectHitbox))
            {                
                if (object->GetType() == static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM))
                {
                    mPlatformId = object->GetEntityId();
                    platformContactDetected = true;
                }
            }
        }
    }

    void UpdateState()
    {

    }

    void Animate(const sf::Time& timeslice)
    {
        UpdateAnimation(timeslice);
        FlipHort(!mIsFacingRight);      
    }

    FloatRect CreateFloorCollider() const
    {
        FloatRect collider = GetHitbox();
        collider.SetHeight(2.0f);
        collider.SetTop(GetHitbox().GetBottom());
        
        return collider;
    }

    FloatRect CreateLeftWallCollider() const
    {
        FloatRect collider = InflateRect(GetHitbox(), 0.0f, -GetHitbox().GetHeight() * 0.5f);
        collider.SetWidth(2.0f);
        collider.SetRight(GetHitbox().GetLeft());

        return collider;
    }

    FloatRect CreateRightWallCollider() const
    {
        FloatRect collider = InflateRect(GetHitbox(), 0.0f, -GetHitbox().GetHeight() * 0.5f);
        collider.SetWidth(2.0f);
        collider.SetLeft(GetHitbox().GetRight());

        return collider;
    }

    void HortCollision()
    {
        for (GameObject* object : mCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsLeftCollision(*object))
                {
                    mHitbox.SetLeft(objectHitbox.GetRight());
                }
                if (IsRightCollision(*object))
                {
                    mHitbox.SetRight(objectHitbox.GetLeft());
                }
            }
        }
    }

    void VertCollision()
    {
        for (GameObject* object : mCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsUpCollision(*object))
                {
                    mHitbox.SetTop(objectHitbox.GetBottom());
                    // TEMP
                    //if (mPlatformId > 0) { mHitbox.MoveY(6); }
                }
                if (IsDownCollision(*object))
                {
                    mHitbox.SetBottom(objectHitbox.GetTop());
                }

                // Prevent velocity from accumulating and player falling through floor
                mDirection.y = 0.0f;
            }
        }
    }

    void SemiCollision()
    {
        // TODO: add timer
        for (GameObject* object : mSemiCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsDownCollision(*object))
                {
                    mHitbox.SetBottom(objectHitbox.GetTop());
                    // Stop player falling through platform when moving down
                    if (mDirection.y > 0.0f) { mDirection.y = 0.0f; }
                }
            }
        }
    }

    FloatRect mHitbox;
    FloatRect mPreviousHitbox;
    Group& mCollisionSprites;
    Group& mSemiCollisionSprites;
    std::string mState;
    sf::Vector2f mDirection;
    float mSpeed;
    float mGravity;
    float mJumpHeight;
    bool mIsJumping;
    bool mIsAttacking;
    bool mIsFacingRight;
    std::unordered_map<std::string, bool> mSurfaceState;
    uint32_t mPlatformId;
};