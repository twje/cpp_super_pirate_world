#include "DrawUtils.h"

void DrawCircle(sf::RenderTarget& target, const sf::Vector2f& position, float radius, const sf::Color& color)
{
	sf::CircleShape circleShape(radius);
	circleShape.setPosition(position - sf::Vector2f(radius, radius));
	circleShape.setFillColor({ 0, 0, 0, 0 });
	circleShape.setOutlineColor(color);
	circleShape.setOutlineThickness(-1);

	target.draw(circleShape);
}