#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <unordered_map>
#include <string>

//------------------------------------------------------------------------------
enum class FontId : uint32_t
{
    DEBUG_FONT = 0
};

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;
constexpr uint32_t ANIMATION_SPEED = 6;

extern std::unordered_map<FontId, std::string> FONT_MAP;