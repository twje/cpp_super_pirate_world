#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/TiledMap.h"

//------------------------------------------------------------------------------
class LevelMap
{
public:
    LevelMap(fs::path mapFilepath)
    {
        mTiledMap = std::make_unique<TiledMap>(mapFilepath);
        mTiledMapRenderer = std::make_unique<TiledMapRenderer>(*mTiledMap);
    }

    void Draw(sf::RenderWindow& window)
    {
        mTiledMapRenderer->Draw(window);
    }

private:
    std::unique_ptr<TiledMap> mTiledMap;
    std::unique_ptr<TiledMapRenderer> mTiledMapRenderer;
};