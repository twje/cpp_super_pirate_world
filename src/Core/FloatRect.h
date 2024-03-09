#pragma once

// Includes
//------------------------------------------------------------------------------
#include<SFML/Graphics.hpp>

//------------------------------------------------------------------------------
class FloatRect
{
public:
    FloatRect() = default;
    FloatRect(const sf::Vector2f& position, const sf::Vector2f& size)
        : mRectangle(position, size)
    { }
    FloatRect(const sf::FloatRect& floatRect)
        : mRectangle(floatRect)
    { }

    operator const sf::FloatRect& () { return mRectangle; }    

    std::optional<FloatRect> FindIntersection(const FloatRect& rectangle) const
    {
        return mRectangle.findIntersection(rectangle.mRectangle);
    }

    std::optional<FloatRect> FindIntersection(sf::FloatRect& rectangle) const
    {
        return mRectangle.findIntersection(rectangle);
    }

    bool ContainsPoint(const sf::Vector2f& point)
    {
        return mRectangle.contains(point);
    }

    float GetLeft() const { return mRectangle.left; }
    float GetTop() const { return mRectangle.top; }
    float GetRight() const { return mRectangle.left + mRectangle.width; }
    float GetBottom() const { return mRectangle.top + mRectangle.height; }
    float GetWidth() const { return mRectangle.width; }
    float GetHeight() const { return mRectangle.height; }
    float GetCenterY() const { return mRectangle.top + mRectangle.height / 2.0f; }
    float GetCenterX() const { return mRectangle.left + mRectangle.width / 2.0f; }
    sf::Vector2f GetSize() const { return { mRectangle.width, mRectangle.height }; }
    sf::Vector2f GetCenter() const { return { GetCenterX(), GetCenterY() }; }
    sf::Vector2f GetPosition() const { return { mRectangle.left, mRectangle.top }; }
    sf::Vector2f GetRoundedPosition() const { return { std::round(mRectangle.left), std::round(mRectangle.top) }; }

    void SetWidth(float value) { mRectangle.width = value; }
    void SetHeight(float value) { mRectangle.height = value; }
    void SetLeft(float value) { mRectangle.left = value; }
    void SetTop(float value) { mRectangle.top = value; }
    void SetRight(float value) { mRectangle.left = value - mRectangle.width; }
    void SetBottom(float value) { mRectangle.top = value - mRectangle.height; }
    void SetPosition(const sf::Vector2f& position)
    {
        mRectangle.left = position.x;
        mRectangle.top = position.y;
    }
    void SetSize(const sf::Vector2f& size)
    {
        mRectangle.width = size.x;
        mRectangle.height = size.y;
    }

    void MoveX(float value) { SetLeft(GetLeft() + value); }
    void MoveY(float value) { SetTop(GetTop() + value); }

    void SetRectMidTop(const sf::Vector2f& value)
    {
        mRectangle.left = value.x - mRectangle.width / 2.0f;
        mRectangle.top = value.y;
    }

    void SetRectMidBottom(const sf::Vector2f& value)
    {
        mRectangle.left = value.x - mRectangle.width / 2.0f;
        mRectangle.top = value.y - mRectangle.height;
    }

    sf::Vector2f GetRectMidBottom() const
    {
        return sf::Vector2f(mRectangle.left + mRectangle.width / 2.0f, mRectangle.top + mRectangle.height);
    }

    sf::Vector2f GetRectMidTop() const
    {
        return sf::Vector2f(mRectangle.left + mRectangle.width / 2.0f, mRectangle.top);
    }

    sf::Vector2f GetRectMidLeft() const
    {
        return sf::Vector2f(mRectangle.left, mRectangle.top + mRectangle.height / 2.0f);
    }

    sf::Vector2f GetRectBottomLeft() const
    {
        return sf::Vector2f(mRectangle.left, mRectangle.top + mRectangle.height);
    }

    sf::Vector2f GetRectBottomRight() const
    {
        return sf::Vector2f(mRectangle.left + mRectangle.width, mRectangle.top + mRectangle.height);
    }

private:
    sf::FloatRect mRectangle;
};