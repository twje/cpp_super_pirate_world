#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"

// Core
#include "Core/TiledMap.h"

//------------------------------------------------------------------------------
class LevelMap
{
public:
    LevelMap(fs::path mapFilepath)
        : mIsDrawObjectLayersEnabled(true)
    {
        mTiledMap = std::make_unique<TiledMap>(mapFilepath);
        mTiledMapRenderer = std::make_unique<TiledMapRenderer>(*mTiledMap);

        SetDrawObjectLayersEnabled(false);
        AddDrawabeLayer("BG", DEPTHS.at("bg tiles"));
        AddDrawabeLayer("FG", DEPTHS.at("bg tiles"));
        AddDrawabeLayer("Terrain", DEPTHS.at("main"));
        AddDrawabeLayer("Platforms", DEPTHS.at("main"));
    }

    const std::map<std::tuple<int32_t, int32_t>, TiledMapTile*>& GetTileDataByLayerName(std::string layerName) const
    {
        for (const TiledMapLayer& layer : mTiledMap->GetLayers())
        {
            if (layer.GetName() == layerName)
            {
                return layer.GetTileData();
            }
        }

        static const std::map<std::tuple<int32_t, int32_t>, TiledMapTile*> emptyVector{ };
        return emptyVector;
    }

    const std::vector<TiledMapObject>& GetObjectsByLayerName(std::string layerName) const
    {
        for (const TiledMapLayer& layer : mTiledMap->GetLayers())
        {
            if (layer.GetName() == layerName)
            {
                return layer.GetObjects();
            }
        }

        static const std::vector<TiledMapObject> emptyVector{ };
        return emptyVector;
    }

    void AddDrawabeLayer(const std::string& layerName, uint32_t depthIndex)
    {
        const std::vector<TiledMapLayer>& layers = mTiledMap->GetLayers();
        
        for (uint32_t index = 0; index < static_cast<uint32_t>(layers.size()); index++)
        {
            if (layerName == layers.at(index).GetName())
            {
                mDrawableLayers[index].push_back(index);
                break;
            }
        }
    }

    void Draw(sf::RenderWindow& window, uint32_t depth)
    {        
        const std::vector<TiledMapLayer>& layers = mTiledMap->GetLayers();

        for (size_t index : mDrawableLayers[depth])
        {
            const TiledMapLayer& layer = layers[index];
            if (layer.GetType() == TiledMapLayerType::ObjectGroup)
            {
                continue;
            }
            mTiledMapRenderer->Draw(window, layer);
        }

        DebugDraw(window, layers);
    }

    const sf::Texture& GetTexture(uint32_t gid) const { return *mTiledMap->GetTetxure(gid); }
    sf::Vector2f GetTileSize() const { return mTiledMap->GetTileSize(); }

    // Draw object layer control
    bool IsDrawObjectLayersEnabled() const { return mIsDrawObjectLayersEnabled; }
    void SetDrawObjectLayersEnabled(bool flag) { mIsDrawObjectLayersEnabled = flag; }
    void ToggleDrawObjectLayersEnabled() { mIsDrawObjectLayersEnabled = !mIsDrawObjectLayersEnabled; }

private:
    void DebugDraw(sf::RenderWindow& window, const std::vector<TiledMapLayer>& layers)
    {
        if (mIsDrawObjectLayersEnabled)
        {
            for (const TiledMapLayer& layer : layers)
            {
                if (layer.GetType() == TiledMapLayerType::ObjectGroup)
                {
                    mTiledMapRenderer->Draw(window, layer);
                }
            }
        }
    }

    std::unique_ptr<TiledMap> mTiledMap;
    std::unique_ptr<TiledMapRenderer> mTiledMapRenderer;
    std::unordered_map<uint32_t, std::vector<uint32_t>> mDrawableLayers;
    bool mIsDrawObjectLayersEnabled;
};