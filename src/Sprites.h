#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "BaseSprites.h"

//------------------------------------------------------------------------------
class AnimatedSpriteImpl : public AnimatedSprite
{
public:
    AnimatedSpriteImpl(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames,
                       uint32_t animSpeed, uint32_t depth)
        : AnimatedSprite(position, scale, animFrames, animSpeed, depth)
    {
        AddAnimationSequence("current", animFrames);
        SetAnimationSequence("current");
    }

    virtual void Update(const sf::Time & timeslice)
    {
        UpdateAnimation(timeslice);
    }
};

//------------------------------------------------------------------------------
class Spike : public GameObject
{
public:
    Spike(const sf::Texture& texture, const sf::Vector2f& position, float radius, float speed,
        float startAngle, float endAngle, uint32_t depth)
        : mSprite(texture)
        , mCenter(position)
        , mRadius(radius)
        , mSpeed(speed)
        , mAngle(startAngle)
        , mStartAngle(startAngle)
        , mEndAngle(endAngle)
        , mDirection(1.0f)
        , mIsFullCircle(endAngle == -1.0f)
        , mDepth(depth)
    {
        SetOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
        UpdatePosition();
    }

    virtual uint32_t GetDepth() const override { return mDepth; }

    virtual FloatRect GetGlobalBounds() const
    {
        return GetTransform().transformRect(mSprite.getLocalBounds());
    }

    virtual void Update(const sf::Time& timeslice)
    {
        mAngle += mDirection * mSpeed * timeslice.asSeconds();

        if (!mIsFullCircle)
        {
            if (mAngle >= mEndAngle) { mDirection = -1.0f; }
            if (mAngle < mStartAngle) { mDirection = 1.0f; }
        }

        UpdatePosition();
    }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

private:
    void UpdatePosition()
    {
        float x = mCenter.x + std::cos(sf::degrees(mAngle).asRadians()) * mRadius;
        float y = mCenter.y + std::sin(sf::degrees(mAngle).asRadians()) * mRadius;
        SetPosition({ x, y });
    }

    sf::Sprite mSprite;
    sf::Vector2f mCenter;
    float mDirection;
    float mRadius;
    float mSpeed;
    float mAngle;
    float mStartAngle;
    float mEndAngle;
    bool mIsFullCircle;
    uint32_t mDepth;
};

//------------------------------------------------------------------------------
class Pearl : public Sprite
{
public:
    Pearl(const sf::Texture& texture, const sf::Vector2f& position, float direction, float speed)
        : Sprite(texture, position, DEPTHS.at("main"))
        , mDirection(direction)
        , mSpeed(speed)
        , mTimer(sf::milliseconds(5000))
    {
        Move({ direction * 50.0f, 0.0f });
        mTimer.Start();
    }

    virtual void Update(const sf::Time& timeslice)
    {        
        float delta = mDirection * mSpeed * timeslice.asSeconds();        
        Move({ delta, 0.0f });

        mTimer.Update(timeslice);
        if (mTimer.IsFinished()) { Kill(); }
    };

private:
    float mDirection;
    float mSpeed;
    Timer mTimer;
};

//------------------------------------------------------------------------------
class Shell : public AnimatedSprite
{
public:
    Shell(const sf::Vector2f& position, bool isReverse, TextureMap& animFrames, uint32_t animSpeed, Player& player, ILevel& levelCallbacks)
        : AnimatedSprite(position, sf::Vector2f(1.0f, 1.0f), animFrames["idle"], animSpeed, DEPTHS.at("main"))
        , mIsReverse(isReverse)
        , mPlayer(player)
        , mLevelCallbacks(levelCallbacks)
        , mState("idle")
        , mBulletDirection(isReverse ? -1.0f : 1.0f)
        , mShootTimer(sf::milliseconds(3000))
        , mHasFired(false)
    {
        mShootTimer.Start();

        for (const auto& [sequenceId, frames] : animFrames)
        {
            AddAnimationSequence(sequenceId, frames);
        }
        SetAnimationSequence(mState);
    }

    virtual void Update(const sf::Time& timeslice)
    {
        mShootTimer.Update(timeslice);

        sf::Vector2f playerPos = mPlayer.GetGlobalBounds().GetCenter();
        sf::Vector2f shellPos = GetGlobalBounds().GetCenter();

        bool isPlayerFront = (mBulletDirection > 0.0f) ? (shellPos.x < playerPos.x) : (shellPos.x > playerPos.x);
        bool isPlayerNear = (shellPos - playerPos).length() < 500;
        bool isPlayerLevel = std::abs(shellPos.y - playerPos.y) < 30.0f;

        if (isPlayerNear && isPlayerFront && isPlayerLevel && mShootTimer.IsFinished())
        {
            mState = "fire";
            mShootTimer.Reset(true);
        }

        if (UpdateAnimation(timeslice))
        {
            if (mState == "fire")
            {
                mState = "idle";
                mHasFired = false;
            }
        }
        else if (mState == "fire" && GetAnimationFrameIndex() == 3 && !mHasFired)
        {
            mLevelCallbacks.CreatePearl(GetGlobalBounds().GetCenter(), mBulletDirection);
            mHasFired = true;
        }

        SetAnimationSequence(mState);

        if (mIsReverse)
        {
            FlipHort(mIsReverse);
        }
    }

private:
    bool mIsReverse;
    Player& mPlayer;
    ILevel& mLevelCallbacks;
    std::string mState;
    float mBulletDirection;
    Timer mShootTimer;
    bool mHasFired;
};

//------------------------------------------------------------------------------
class Tooth : public AnimatedSpriteImpl
{
public:
    Tooth(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, 
          uint32_t animSpeed, Group& collisionSprites)
        : AnimatedSpriteImpl(position, scale, animFrames, animSpeed, DEPTHS.at("main"))
        , mCollisionSprites(collisionSprites)
        , mDirection(1.0f)
        , mSpeed(200.0f)
    {
        mHitbox = GetGlobalBounds();
    }

    virtual FloatRect GetHitbox() const { return mHitbox; }

    virtual void Update(const sf::Time& timeslice)
    {
        float delta = mDirection * mSpeed * timeslice.asSeconds();
        mHitbox.MoveX(delta);

        sf::Vector2f floorCollider = CreateFloorCollider();
        sf::FloatRect wallCollider = CreateWallCollider();
        
        if (ShouldReverseDir(floorCollider, wallCollider))
        {
            mDirection *= -1.0f;
            FlipHort(mDirection < 0.0f);
        }

        UpdateAnimation(timeslice);   
        SetPosition(mHitbox.GetRoundedPosition());
    }

private:
    sf::Vector2f CreateFloorCollider()
    {
        sf::Vector2f floorCollider = mDirection > 0.0f ? mHitbox.GetRectBottomRight() : mHitbox.GetRectBottomLeft();
        floorCollider.y += 1;

        return floorCollider;
    }

    sf::FloatRect CreateWallCollider()
    {        
        sf::FloatRect wallColider = InflateRect(mHitbox, 2, 0);
        wallColider.height = 1;
        
        return wallColider;
    }

    bool ShouldReverseDir(const sf::Vector2f& floorCollider, const sf::FloatRect& wallCollider) const 
    {        
        bool onFloor = false;
        bool hitWall = false;

        for (GameObject* object : mCollisionSprites)
        {
            if (object->GetHitbox().ContainsPoint(floorCollider))
            {
                onFloor = true;
            }
            if (object->GetHitbox().FindIntersection(wallCollider))
            {
                hitWall = true;
                break;
            }
        }

        return hitWall || !onFloor;
    }

    Group& mCollisionSprites;
    float mDirection;
    float mSpeed;
    FloatRect mHitbox;
    sf::FloatRect mTemp;
};

//------------------------------------------------------------------------------
class Item : public AnimatedSpriteImpl
{
public:
    Item(const std::string& itemType, const sf::Vector2f& position, const sf::Vector2f& scale,
        TextureVector& animFrames, uint32_t animSpeed, GameData& gameData)
        : AnimatedSpriteImpl(position, scale, animFrames, animSpeed, DEPTHS.at("main"))
        , mItemType(itemType)
        , mGameData(gameData)
    {
        SetOrigin(sf::Vector2f(animFrames[0]->getSize()) * 0.5f);
    }

    void Activate()
    {
        if (mItemType == "gold") { mGameData.AddCoins(5); }
        else if (mItemType == "silver") { mGameData.AddCoins(1); }
        else if (mItemType == "diamond") { mGameData.AddCoins(20); }
        else if (mItemType == "skull") { mGameData.AddCoins(50); }
        else if (mItemType == "potion") { mGameData.AddHealth(1); }
    }

private:
    std::string mItemType;
    GameData& mGameData;
};

//------------------------------------------------------------------------------
class MovingSprite : public AnimatedSpriteImpl
{
public:
    MovingSprite(const sf::Vector2f& startPos, const sf::Vector2f& endPos, bool isVertMovement, int32_t speed,
        const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed, bool isFlippable)
        : AnimatedSpriteImpl(startPos, scale, animFrames, animSpeed, DEPTHS.at("main"))
        , mStartPos(startPos)
        , mEndPos(endPos)
        , mIsVertMovement(isVertMovement)
        , mSpeed(speed)
        , mIsFlippable(isFlippable)
        , mIsVertReverseDir(false)
        , mIsHortReverseDir(false)
    {
        UpdateOrigin(*animFrames[0]);
        mDirection = isVertMovement ? sf::Vector2f(0.0f, 1.0f) : sf::Vector2f(1.0f, 0.0f);
        mHitbox = GetGlobalBounds();
        mPreviousHitbox = mHitbox;
    }

    const sf::Vector2f& GetDirection() const { return mDirection; }
    float GetSpeed() const { return mSpeed; }

    virtual uint32_t GetType() const 
    { 
        return static_cast<uint32_t>(SpritTypes::MOVING_PLATFORM); 
    }

    virtual FloatRect GetHitbox() const override
    {
        return mHitbox;
    }

    virtual FloatRect GetPreviousHitbox() const override
    {
        return mPreviousHitbox;
    }

    virtual const sf::Vector2f GetVelocity() const  override
    { 
        return mVelocity;
    };

    virtual void Update(const sf::Time& timeslice)
    {
        mPreviousHitbox = mHitbox;
        mVelocity = mDirection * mSpeed * timeslice.asSeconds();
        sf::Vector2f newPosition = mHitbox.GetPosition() + mVelocity;
        mHitbox.SetPosition(newPosition);

        if (mIsVertMovement)
        {
            if (mHitbox.GetBottom() >= mEndPos.y && mDirection.y == 1.0f)
            {
                mDirection.y = -1.0f;
                mHitbox.SetBottom(mEndPos.y);
            }
            if (mHitbox.GetTop() <= mStartPos.y && mDirection.y == -1.0f)
            {
                mDirection.y = 1.0f;
                mHitbox.SetTop(mStartPos.y);
            }
            mIsVertReverseDir = mDirection.y > 0.0f;
        }
        else
        {
            if (mHitbox.GetRight() >= mEndPos.x && mDirection.x == 1.0f)
            {
                mDirection.x = -1.0f;
                mHitbox.SetRight(mEndPos.x);
            }
            if (mHitbox.GetLeft() <= mStartPos.x && mDirection.x == -1.0f)
            {
                mDirection.x = 1.0f;
                mHitbox.SetLeft(mStartPos.x);
            }
            mIsHortReverseDir = mDirection.x < 0.0f;
        }

        sf::Vector2f center = mHitbox.GetCenter();
        SetPosition({ std::round(center.x), std::round(center.y) });

        UpdateAnimation(timeslice);

        if (mIsFlippable)
        {
            FlipHort(mIsHortReverseDir);
            FlipVert(mIsVertReverseDir);
        }
    }

private:
    void UpdateOrigin(const sf::Texture& texture)
    {
        SetOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
    }

    sf::Vector2f mStartPos;
    sf::Vector2f mEndPos;
    bool mIsVertMovement;
    float mSpeed;
    bool mIsFlippable;
    bool mIsVertReverseDir;
    bool mIsHortReverseDir;
    sf::Vector2f mDirection;
    sf::Vector2f mVelocity;
    FloatRect mHitbox;
    FloatRect mPreviousHitbox;
};
