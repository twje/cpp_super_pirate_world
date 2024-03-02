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

    const std::vector<TiledMapObject>& GetObjectsByLayerName(std::string layerName) const
    { 
        return mTiledMap->GetObjectsByLayerName(layerName); 
    }

    bool GetDrawObjectLayers() const { return mTiledMapRenderer->GetDrawObjectLayers(); }
    void SetDrawObjectLayers(bool flag) { mTiledMapRenderer->SetDrawObjectLayers(flag); }
    void ToggleDrawObjectLayers() { mTiledMapRenderer->ToggleDrawObjectLayers(); }

    void Draw(sf::RenderWindow& window)
    {
        mTiledMapRenderer->Draw(window);
    }

private:
    std::unique_ptr<TiledMap> mTiledMap;
    std::unique_ptr<TiledMapRenderer> mTiledMapRenderer;
};