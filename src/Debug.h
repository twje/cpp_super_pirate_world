#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"

// Third party
#include <SFML/Graphics.hpp>

// Core
#include "Core/ResourceManager.h"

//------------------------------------------------------------------------------
void DrawText(sf::RenderTarget& target, FontId fontId, const sf::String& value, const sf::Vector2f& position)
{
    ResourceLocator& locator = ResourceLocator::GetInstance();
    sf::Font* font = locator.GetFontManager().GetResource(FONT_MAP.at(FontId::DEBUG_FONT));
    sf::Text text(*font, value, 15);
    text.setPosition(position);
    target.draw(text);
}