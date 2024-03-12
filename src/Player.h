#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "BaseSprites.h"


// Core
#include "Core/DrawUtils.h"
#include "Core/GameObjectManager.h"
#include "Core/Timer.h"

//------------------------------------------------------------------------------
class PlayerState
{
public:
    virtual void Input() = 0;
    virtual void Update(const sf::Time& timeslice) = 0;


};

//------------------------------------------------------------------------------
class Standing : public PlayerState
{
public:
    virtual void Input() override
    {

    }

    virtual void Update(const sf::Time& timeslice) override
    {

    }
};

//------------------------------------------------------------------------------
class PassingThroughPlatform : public PlayerState
{
public:
    virtual void Input() override
    {

    }

    virtual void Update(const sf::Time& timeslice) override
    {

    }
};

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
        , mIsJumping(false)
        , mIsAttacking(false)
        , mPrvSpaceKeyPress(false)
        , mCntSpaceKeyPress(false)
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

        mTimers = {
            { "wall jump", Timer(sf::milliseconds(400)) },
            { "wall slide block", Timer(sf::milliseconds(250)) },
            { "platform skip", Timer(sf::milliseconds(250)) },
        };

        SetOrigin(GetGlobalBounds().GetSize() * 0.5f);
        GameObject::Move(GetGlobalBounds().GetSize() * 0.5f);
        mHitbox = InflateRect(GetGlobalBounds(), -76, -36);
        mPreviousHitbox = mHitbox;
    }

    virtual FloatRect GetHitbox() const override
    {
        return mHitbox;
    }

    virtual FloatRect GetPreviousHitbox() const override
    {
        return mPreviousHitbox;
    }

    virtual void Update(const sf::Time& timeslice)
    {        
        mPreviousHitbox = mHitbox;
        
        UpdateTimers(timeslice);
        Input();
        Move(timeslice);
    };
   
    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {        
        DrawRect<float>(target, CreateLeftWallCollider(), sf::Color::Green);
        DrawRect<float>(target, CreateRightWallCollider(), sf::Color::Green);
        DrawRect<float>(target, GetHitbox(), sf::Color::Red);
        Sprite::draw(target, states);

        FloatRect floorIndicator(sf::Vector2f(10, 10), sf::Vector2f(20, 20));
        sf::Color color = sf::Color::Red;
        if (mSurfaceState.at("floor"))
        {
            sf::Color color = sf::Color::Green;
        }
        DrawRect<float>(target, floorIndicator, color);
    }

    sf::Vector2f GetCameraCenter() { return GetGlobalBounds().GetCenter(); }

private:
    void UpdateTimers(const sf::Time& timeslice)
    {
        for (auto& [_, timer] : mTimers)
        {
            timer.Update(timeslice);
            if (timer.IsFinished())
            {
                timer.Reset(false);
            }
        }
    }

    void Input()
    {
        if (!mTimers.at("wall jump").IsActive())
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                mDirection.x = 1.0f;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                mDirection.x = -1.0f;
            }
            else
            {
                mDirection.x = 0.0f;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                mTimers.at("platform skip").Start();
            }
        }

        mPrvSpaceKeyPress = mCntSpaceKeyPress;
        mCntSpaceKeyPress = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
 
        if (mCntSpaceKeyPress && !mPrvSpaceKeyPress)
        {
            mIsJumping = true;
        }
    }    

    void Move(const sf::Time& timeslice)
    {              
        // Vert movement
        if (IsWallSliding() && !mTimers.at("wall slide block").IsActive())
        {
            mHitbox.MoveY(mGravity / 10.0f * timeslice.asSeconds());
            mDirection.y = 0.0f;
        }
        else
        {
            mDirection.y += mGravity * 0.5f * timeslice.asSeconds();
            // https://stackoverflow.com/questions/60198718/gravity-strength-and-jump-height-somehow-dependant-on-framerate-pygame
            // This step accounts for the distance the object travels while accelerating
            mHitbox.MoveY(mDirection.y * timeslice.asSeconds());
            mDirection.y += mGravity * 0.5f * timeslice.asSeconds();
        }

        CheckAndResolveVertCollision();

        float deltaX = mDirection.x * mSpeed * timeslice.asSeconds();
        if (GameObject* platform = GetPlatform())
        {
            // Collided with a platform while falling this frame; on the ground the previous frame
            if (mDirection.y > 0 && mSurfaceState["floor"])
            {
                // Stick player to descending platform
                mHitbox.SetBottom(platform->GetHitbox().GetTop());
            }
            
            FloatRect platformHitbox = platform->GetHitbox();
            FloatRect platformPreviousHitbox = platform->GetPreviousHitbox();
            deltaX += platformHitbox.GetLeft() - platformPreviousHitbox.GetLeft(); 
        }

        CheckContacts();

        if (mIsJumping)
        {
            if (IsWallSliding() && !mTimers.at("wall slide block").IsActive())
            {
                mTimers.at("wall jump").Start();  // Temporarily disable user input
                mDirection.y = -mJumpHeight;
                mDirection.x = mSurfaceState.at("left") ? 1.0f : -1.0f;  // Bounce off wall in opposite direction              
                mTimers.at("wall slide block").Start();  // Temporarily disable wall slide 
            }
            else if (mSurfaceState["floor"])
            {
                mHitbox.MoveY(-1);            
                mDirection.y = -mJumpHeight;
                mTimers.at("wall slide block").Start();  // Temporarily disable wall slide
            }
            mIsJumping = false;
        }

        // Hort movement
        mHitbox.MoveX(deltaX);
        CheckAndResolveHortCollision();

        SetPosition(mHitbox.GetCenter());
    }

    GameObject* GetPlatform()
    {
        if (mTimers.at("platform skip").IsActive())
        {
            return nullptr;
        }

        FloatRect floorCollider = CreateFloorCollider();

        for (GameObject* object : mSemiCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (floorCollider.FindIntersection(objectHitbox))
            {
                bool isPlatform = (object->GetType() == static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM));
                if (isPlatform)
                {
                    return object;
                }
            }
        }
        return nullptr;
    }

    void CheckContacts()
    {
        mSurfaceState["left"] = false;
        mSurfaceState["right"] = false;
        mSurfaceState["floor"] = false;

        FloatRect leftWallCollider = CreateLeftWallCollider();
        FloatRect rightWallCollider = CreateRightWallCollider();
        FloatRect floorCollider = CreateFloorCollider();

        for (GameObject* object : mCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (leftWallCollider.FindIntersection(objectHitbox))
            {
                mSurfaceState["left"] = true;
                break;
            }
            if (rightWallCollider.FindIntersection(objectHitbox))
            {
                mSurfaceState["right"] = true;
                break;
            }
            if (floorCollider.FindIntersection(objectHitbox))
            {                
                 mSurfaceState["floor"] = true;
                break;
            }
        }

        for (GameObject* object : mSemiCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (floorCollider.FindIntersection(objectHitbox))
            {
                mSurfaceState["floor"] = true;
                break;
            }
        }
    }

    void Animate(const sf::Time& timeslice)
    {
        UpdateAnimation(timeslice);
        FlipHort(mDirection.x > 0.0f);
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

    FloatRect CreateFloorCollider() const
    {
        FloatRect collider = GetHitbox();
        collider.SetHeight(2.0f);
        collider.SetTop(GetHitbox().GetBottom());

        return collider;
    }

    void CheckAndResolveHortCollision()
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

    void CheckAndResolveVertCollision()
    {
        for (GameObject* object : mCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsUpCollision(*object))
                {
                    mHitbox.SetTop(objectHitbox.GetBottom());
                    mDirection.y = 0.0f;
                }
                if (IsDownCollision(*object))
                {
                    mHitbox.SetBottom(objectHitbox.GetTop());
                    mDirection.y = 0.0f;
                }
            }
        }

        for (GameObject* object : mSemiCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsDownCollision(*object) && !mTimers.at("platform skip").IsActive())
                {
                    bool isPlatform = (object->GetType() == static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM));
                    if (isPlatform && mTimers.at("platform skip").IsActive())
                    {
                        continue;
                    }

                    mHitbox.SetBottom(objectHitbox.GetTop());
                    mDirection.y = 0.0f;
                }
            }
        }
    }

    bool IsWallSliding() { return !mSurfaceState["floor"] && (mSurfaceState["left"] || mSurfaceState["right"]); }

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
    bool mPrvSpaceKeyPress = false;
    bool mCntSpaceKeyPress = false;
    std::unordered_map<std::string, bool> mSurfaceState;
    std::unordered_map<std::string, Timer> mTimers;
};