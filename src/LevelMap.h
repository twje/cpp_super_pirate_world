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
    const sf::Texture& GetTexture(uint32_t gid) const { return *mTiledMap->GetTetxure(gid); }
    sf::Vector2f GetTileSize() const { return mTiledMap->GetTileSize(); }

    void Draw(sf::RenderWindow& window)
    {
        mTiledMapRenderer->Draw(window);
    }

private:
    std::unique_ptr<TiledMap> mTiledMap;
    std::unique_ptr<TiledMapRenderer> mTiledMapRenderer;
};