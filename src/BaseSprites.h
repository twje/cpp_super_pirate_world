#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"
#include "GameData.h"
#include "Interfaces.h"
#include "Player.h"

// Core
#include "Core/GameObject.h"
#include "Core/Animate.h"
#include "Core/ResourceManager.h"
#include "Core/Timer.h"
#include "Core/DrawUtils.h"
#include "Core/RectUtils.h"

// System
#include <cmath>
#include <iostream>

//------------------------------------------------------------------------------
class Sprite : public GameObject
{
public:
    Sprite(const sf::Texture& texture, const sf::IntRect& textureRegion, const sf::Vector2f& position, uint32_t depth)
        : mSprite(texture, textureRegion)
        , mDepth(depth)
    {
        SetPosition(position);
    }

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

    virtual uint32_t GetDepth() const override
    {
        return mDepth;
    }

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

    void SetTextureRegion(const sf::IntRect& region)
    {
        mSprite.setTextureRect(region);
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
    sf::Sprite mSprite;
    uint32_t mDepth;
};

//------------------------------------------------------------------------------
class AnimatedSprite : public Sprite
{
public:
    AnimatedSprite(const sf::Vector2f& position, const sf::Vector2f& scale, TextureVector& animFrames,
        uint32_t animSpeed, uint32_t depth)
        : Sprite(*animFrames[0], position, depth)
        , mAnimation(animSpeed)
    {
        SetScale(scale);
    }

    bool UpdateAnimation(const sf::Time& timeslice)
    {
        bool isRestarted = mAnimation.Update(timeslice);
        SetTexture(mAnimation.GetTexture(), true);
        return isRestarted;
    }

    void AddAnimationSequence(const std::string& sequenceId, const TextureVector& frames)
    {
        auto animFramesCopy = std::make_unique<std::vector<sf::Texture*>>();
        for (auto& texturePtr : frames)
        {
            animFramesCopy->push_back(&(*texturePtr));
        }
        mAnimation.AddSequence({ sequenceId, std::move(animFramesCopy) });
    }

    void SetAnimationSequence(const std::string& sequenceId)
    {
        mAnimation.SetSequence(sequenceId);
        SetTexture(mAnimation.GetTexture(), true);
    }

    uint32_t GetAnimationFrameIndex() { return mAnimation.GetFrameIndex(); }

private:
    Animation mAnimation;
};