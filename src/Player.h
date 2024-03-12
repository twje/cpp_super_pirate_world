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

        mTimers = {
            { "wall jump", Timer(sf::milliseconds(400)) },
            { "wall slide block", Timer(sf::milliseconds(250)) },
            { "platform skip", Timer(sf::milliseconds(100)) },
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
        CheckWallContact();
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

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            {
                mTimers.at("platform skip").Start();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            mIsJumping = true;
        }
    }    

    void Move(const sf::Time& timeslice)
    {        
        // Handle moving platform
        sf::Vector2f previousFramePlatformVelocity;
        float deltaX = 0.0f;

        GameObject* platform = GameObjectManager::Instance().GetGameObject(mPlatformId);
        if (platform && mSurfaceState["floor"])
        {
            previousFramePlatformVelocity = platform->GetHitbox().GetPosition() - platform->GetPreviousHitbox().GetPosition();

            if (previousFramePlatformVelocity.y > 0.0f)
            {
                mDirection.y += previousFramePlatformVelocity.y;
                mHitbox.SetBottom(platform->GetHitbox().GetTop());
            }
            deltaX += previousFramePlatformVelocity.x;
        }

        // Vert movement
        if (IsWallSliding() && !mTimers.at("wall slide block").IsActive())
        {
            mDirection.y = 0.0f;
            mHitbox.MoveY(mGravity / 10.0f * timeslice.asSeconds());
        }
        else
        {
            mDirection.y += mGravity * 0.5f * timeslice.asSeconds();
            // https://stackoverflow.com/questions/60198718/gravity-strength-and-jump-height-somehow-dependant-on-framerate-pygame
            // This step accounts for the distance the object travels while accelerating
            mHitbox.MoveY(mDirection.y * timeslice.asSeconds());
            mDirection.y += mGravity * 0.5f * timeslice.asSeconds(); // TODO: investigate
        }

        if (mIsJumping)
        {
            // Wall jump
            if (IsWallSliding() && !mTimers.at("wall slide block").IsActive())
            {
                mTimers.at("wall jump").Start();  // Temporarily disable user input
                mDirection.y = -mJumpHeight;
                mDirection.x = mSurfaceState.at("left") ? 1.0f : -1.0f;  // Bounce off wall in opposite direction
            }
            // Ordinary jump
            else if (mSurfaceState["floor"])
            {
                // Unstick from floor or platform to enable jump
                mHitbox.MoveY(-1);
                if (previousFramePlatformVelocity.y < 0.0f)
                {
                    mHitbox.MoveY(previousFramePlatformVelocity.y);
                }
                mDirection.y = -mJumpHeight;
                mTimers.at("wall slide block").Start();
            }
            mIsJumping = false;
        }
           
        CheckAndResolveVertCollision();

        // Hort movement
        deltaX += mDirection.x * mSpeed * timeslice.asSeconds();
        mHitbox.MoveX(deltaX);

        CheckAndResolveHortCollision();

        SetPosition(mHitbox.GetCenter());
    }

    void CheckWallContact()
    {
        mSurfaceState["left"] = false;
        mSurfaceState["right"] = false;

        FloatRect leftWallCollider = CreateLeftWallCollider();
        FloatRect rightWallCollider = CreateRightWallCollider();

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
        }
    }

    void Animate(const sf::Time& timeslice)
    {
        UpdateAnimation(timeslice);
        FlipHort(!mIsFacingRight);
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
        mPlatformId = false;
        mSurfaceState["floor"] = false;
        std::vector<const GameObject*> objectsBelow;
        std::vector<const GameObject*> objectsAbove;

        for (GameObject* object : mCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsUpCollision(*object))
                {
                    objectsAbove.emplace_back(object);
                }
                if (IsDownCollision(*object))
                {
                    objectsBelow.emplace_back(object);
                }
            }
        }

        for (GameObject* object : mSemiCollisionSprites)
        {
            FloatRect objectHitbox = object->GetHitbox();
            if (mHitbox.FindIntersection(objectHitbox))
            {
                if (IsUpCollision(*object))
                {
                    objectsAbove.emplace_back(object);
                }
                if (IsDownCollision(*object))
                {
                    objectsBelow.emplace_back(object);
                }
            }
        }
        ResolveVertCollision(objectsBelow, objectsAbove);
    }

    void ResolveVertCollision(std::vector<const GameObject*>& objectsBelow, std::vector<const GameObject*>& objectsAbove)
    {        
        // Resolve floor objects
        const GameObject* lowestYVelocityoObject = nullptr;
        for (const GameObject* object : objectsBelow)
        {    
            bool isPlatform = (object->GetType() == static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM));
            if (isPlatform && mTimers.at("platform skip").IsActive())
            {
                continue;
            }

            if (!lowestYVelocityoObject || object->GetVelocity().y < lowestYVelocityoObject->GetVelocity().y)
            {
                lowestYVelocityoObject = object;  
                if (isPlatform)
                {
                    mPlatformId = lowestYVelocityoObject->GetEntityId();
                }
            }
        }

        if (lowestYVelocityoObject)
        {
            mHitbox.SetBottom(lowestYVelocityoObject->GetHitbox().GetTop());
            mDirection.y = 0;
            mSurfaceState["floor"] = true;
        }

        // Resolve ceiling objects
        const GameObject* highestYVelocityObject = nullptr;
        for (const GameObject* object : objectsAbove)
        {
            if (!highestYVelocityObject || object->GetVelocity().y > highestYVelocityObject->GetVelocity().y)
            {
                highestYVelocityObject = object;
            }
        }

        if (highestYVelocityObject)
        {
            // Allow jump through platforms
            if (highestYVelocityObject->GetType() != static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM))
            {
                mHitbox.SetTop(highestYVelocityObject->GetHitbox().GetBottom());
                mDirection.y = highestYVelocityObject->GetVelocity().y;
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
    bool mIsFacingRight;
    std::unordered_map<std::string, bool> mSurfaceState;
    std::unordered_map<std::string, Timer> mTimers;
    uint32_t mPlatformId;
};