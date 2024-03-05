// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"
#include "GameData.h"
#include "Interfaces.h"

// Core
#include "Core/GameObject.h"
#include "Core/Animate.h"
#include "Core/ResourceManager.h"
#include "Core/Timer.h"

// System
#include <cmath>

//------------------------------------------------------------------------------
class Sprite : public GameObject
{
public:
    Sprite(const sf::Texture& texture, const sf::Vector2f& position, uint32_t depth)
        : mSprite(texture)
        , mDepth(depth)
    {
        SetPosition(position);
    }

    virtual FloatRect GetGlobalBounds() const
    {
        return GetTransform().transformRect(mSprite.getLocalBounds());
    }

    virtual uint32_t GetDepth() const override { return mDepth; }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

    void SetTexture(const sf::Texture& texture, bool resetRect)
    {
        mSprite.setTexture(texture, resetRect);
    }

    void FlipHort(bool flag)
    {
        // Transform is applied in local space
        float scaleX = flag ? -1.0f : 1.0f;
        float posX = flag ? mSprite.getLocalBounds().width : 0.0f;

        mSprite.setScale({ scaleX, mSprite.getScale().y });
        mSprite.setPosition({ posX, mSprite.getPosition().y });
    }

private:
    sf::Sprite mSprite;
    uint32_t mDepth;
};

//------------------------------------------------------------------------------
class AnimatedSprite : public GameObject
{
public:
    AnimatedSprite(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames,
        uint32_t animSpeed, uint32_t depth)
        : mAnimation(animSpeed)
        , mSprite(*animFrames[0])
        , mDepth(depth)
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

    virtual uint32_t GetDepth() const override { return mDepth; }

    virtual FloatRect GetGlobalBounds() const
    {
        return GetTransform().transformRect(mSprite.getLocalBounds());
    }

    virtual void Update(const sf::Time& timeslice)
    {
        UpdateAnimation(timeslice);
    }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

    void UpdateAnimation(const sf::Time& timeslice)
    {
        mAnimation.Update(timeslice);
        mSprite.setTexture(mAnimation.GetTexture(), true);
    }

    void FlipHort(bool flag)
    {
        // Transform is applied in local space
        float scaleX = flag ? -1.0f : 1.0f;
        float posX = flag ? mSprite.getLocalBounds().width : 0.0f;

        mSprite.setScale({ scaleX, mSprite.getScale().y });
        mSprite.setPosition({ posX, mSprite.getPosition().y });
    }

    void FlipVert(bool flag)
    {
        // Transform is applied in local space
        float scaleY = flag ? -1.0f : 1.0f;
        float posY = flag ? mSprite.getLocalBounds().height : 0.0f;

        mSprite.setScale({ mSprite.getScale().x, scaleY });
        mSprite.setPosition({ mSprite.getPosition().x, posY });
    }

private:
    Animation mAnimation;
    sf::Sprite mSprite;
    uint32_t mDepth;
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
class Shell : public Sprite
{
public:
    Shell(const sf::Vector2f& position, bool isReverse, TextureMap& animFrames, uint32_t animSpeed, ILevel& levelCallbacks)
        : Sprite(*animFrames.at("idle")[0], position, DEPTHS.at("main"))
        , mIsReverse(isReverse)
        , mAnimation(animSpeed)
        , mLevelCallbacks(levelCallbacks)
        , mState("idle")
        , mBulletDirection(isReverse ? -1.0f : 1.0f)
        , mShootTimer(sf::milliseconds(3000))
        , mHasFired(false)
    { 
        mShootTimer.Start();
        InitAnimation(animFrames, mState);
    }

    virtual void Update(const sf::Time& timeslice)
    {
        mShootTimer.Update(timeslice);

        bool isPlayerNear = true;
        bool isPlayerFront = true;
        bool isPlayerLevel = true;

        if (isPlayerNear && isPlayerFront && isPlayerLevel && mShootTimer.IsFinished())
        {
            mState = "fire";
            mShootTimer.Reset(true);
        }

        if (mAnimation.Update(timeslice))
        {
            if (mState == "fire") 
            { 
                mState = "idle"; 
                mHasFired = false;
            }
        }
        else if (mState == "fire" && mAnimation.GetFrameIndex() == 3 && !mHasFired)
        {
            mLevelCallbacks.CreatePearl();
            mHasFired = true;
        }
        
        mAnimation.SetSequence(mState);
        SetTexture(mAnimation.GetTexture(), true);
        if (mIsReverse)
        {
            FlipHort(mIsReverse);
        }
    }

public:
    void InitAnimation(TextureMap& animFrames, const std::string& startSequenceId)
    {
        for (const auto& [sequenceId, frames] : animFrames)
        {
            auto animFramesCopy = std::make_unique<std::vector<sf::Texture*>>();
            for (auto& texturePtr : frames)
            {
                animFramesCopy->push_back(&(*texturePtr));
            }
            mAnimation.AddSequence({ sequenceId, std::move(animFramesCopy) });
        }
        mAnimation.SetSequence(startSequenceId);
    }

    bool mIsReverse;
    std::string mState;
    ILevel& mLevelCallbacks;
    float mBulletDirection;
    Timer mShootTimer;
    Animation mAnimation;
    bool mHasFired;
};

//------------------------------------------------------------------------------
class Tooth : public AnimatedSprite
{
public:
    Tooth(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
        : AnimatedSprite(position, scale, animFrames, animSpeed, DEPTHS.at("main"))
    { }
};

//------------------------------------------------------------------------------
class Item : public AnimatedSprite
{
public:
    Item(const std::string& itemType, const sf::Vector2f& position, const sf::Vector2f& scale, 
         TextureVector& animFrames, uint32_t animSpeed, GameData& gameData)
        : AnimatedSprite(position, scale, animFrames, animSpeed, DEPTHS.at("main"))
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
class MovingSprite : public AnimatedSprite
{
public:
    MovingSprite(const sf::Vector2f& startPos, const sf::Vector2f& endPos, bool isVertMovement, int32_t speed,
                 const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed, bool isFlippable)
        : AnimatedSprite(startPos, scale, animFrames, animSpeed, DEPTHS.at("main"))
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
        sf::Vector2f newPosition = mHitbox.GetPosition() + mDirection * mSpeed * timeslice.asSeconds();
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
    FloatRect mHitbox;
    FloatRect mPreviousHitbox;
};
