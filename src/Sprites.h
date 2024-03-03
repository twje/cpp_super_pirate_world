// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/GameObject.h"
#include "Core/Animate.h"

// System
#include <cmath> 

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
class Spike : public GameObject
{
public:
    Spike(const sf::Texture& texture, const sf::Vector2f& position, int32_t radius, uint32_t speed,
        int32_t startAngle, int32_t endAngle)
        : mSprite(texture)
        , mCenter(position)
        , mRadius(radius)
        , mSpeed(speed)
        , mAngle(startAngle)
        , mStartAngle(startAngle)
        , mEndAngle(endAngle)
        , mDirection(1.0f)
        , mIsFullCircle(endAngle == -1.0f)
    {
        SetOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
        UpdatePosition();
    }

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
        OnAnimationUpdate(mAnimation.GetTexture());
    }

    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        sf::RenderStates statesCopy(states);
        statesCopy.transform *= GetTransform();
        target.draw(mSprite, statesCopy);
    }

    virtual void OnAnimationUpdate(const sf::Texture& texture) { }

private:
    Animation mAnimation;
    sf::Sprite mSprite;
};

//------------------------------------------------------------------------------
class Shell : public AnimatedSprite
{
public:
    Shell(const sf::Vector2f& position, const sf::Vector2f& scale, TextureMap& animFrames, uint32_t animSpeed)
        : AnimatedSprite(position, scale, animFrames.at("idle"), animSpeed)
        , mState("idle")
    { }

public:
    std::string mState;
};

//------------------------------------------------------------------------------
class Tooth : public AnimatedSprite
{
public:
    Tooth(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
        : AnimatedSprite(position, scale, animFrames, animSpeed)
    { }
};

//------------------------------------------------------------------------------
class Item : public AnimatedSprite
{
public:
    Item(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
        : AnimatedSprite(position, scale, animFrames, animSpeed)
    {
        SetOrigin(sf::Vector2f(animFrames[0]->getSize()) * 0.5f);
    }

    virtual void OnAnimationUpdate(const sf::Texture& texture) override
    {
        SetOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
    }
};

//------------------------------------------------------------------------------
class MovingSprite : public AnimatedSprite
{
public:
    MovingSprite(const sf::Vector2f& startPos, const sf::Vector2f& endPos, bool isVertMovement, int32_t speed,
        const sf::Vector2f& scale, TextureVector& animFrames, uint32_t animSpeed)
        : AnimatedSprite(startPos, scale, animFrames, animSpeed)
        , mIsVertMovement(isVertMovement)
    {
        UpdateOrigin(*animFrames[0]);
    }

    virtual void Update(const sf::Time& timeslice)
    {
        // implement
    }

    virtual void OnAnimationUpdate(const sf::Texture& texture) override
    {
        UpdateOrigin(texture);
    }

private:
    void UpdateOrigin(const sf::Texture& texture)
    {
        SetOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
    }

    bool mIsVertMovement;
};
