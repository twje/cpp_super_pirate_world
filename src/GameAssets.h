#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Settings.h"

// Core
#include "Core/ResourceManager.h"

//------------------------------------------------------------------------------
class GameAssets
{
    using AssetLookup = std::unordered_map<std::string, std::string>;

public:
    void LoadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().RequireResource(filepath);
        }

        // Textures organised by sub-directories        
        for (auto& [id, filepath] : GetTextureDirMapsLookup())
        {
            locator.GetTextureDirMapManager().RequireResource(filepath);
        }

        // Textures in directory
        for (auto& [id, filepath] : GetTextureVecMapLookup())
        {
            locator.GetTextureVectorManager().RequireResource(filepath);
        }
    }

    void UnloadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().ReleaseResource(filepath);
        }

        // Textures organised by sub-directories
        for (auto& [_, filepath] : GetTextureDirMapsLookup())
        {
            locator.GetTextureDirMapManager().ReleaseResource(filepath);
        }        

        // Textures in directory
        for (auto& [_, filepath] : GetTextureVecMapLookup())
        {
            locator.GetTextureVectorManager().ReleaseResource(filepath);
        }
    }

    TextureMap& GetTextureDirMap(const std::string& id) const 
    { 
        ResourceLocator& locator = ResourceLocator::GetInstance();
        AssetLookup& lookup = GetTextureDirMapsLookup();
        assert(lookup.count(id) > 0);

        return *locator.GetTextureDirMapManager().GetResource(lookup.at(id));
    }
    
    TextureVector& GetTextureVecMap(const std::string& id) const 
    { 
        ResourceLocator& locator = ResourceLocator::GetInstance();
        AssetLookup& lookup = GetTextureVecMapLookup();
        assert(lookup.count(id) > 0);

        return *locator.GetTextureVectorManager().GetResource(lookup.at(id));
    }

private:
    static AssetLookup& GetTextureDirMapsLookup()
    {
        static AssetLookup lookup = { 
            {"palms", "graphics/level/palms"} 
        };
        
        return lookup;
    }

    static AssetLookup& GetTextureVecMapLookup()
    {
        static AssetLookup lookup = { 
            {"flag", "graphics/level/flag"},
            {"saw", "graphics/enemies/saw/animation"},
            {"floor_spike", "graphics/enemies/floor_spikes"},
            {"candle", "graphics/level/candle"},
            {"window", "graphics/level/window"},
            {"big_chain", "graphics/level/big_chains"},
            {"small_chain", "graphics/level/small_chains"},
            {"candle_light", "graphics/level/candle light"},
            {"saw", "enemies/saw/animation"},
            {"helicopter", "graphics/level/helicopter"},
            {"boat", "graphics/objects/boat"},
            {"tooth", "graphics/enemies/tooth/run"},
            {"particle", "graphics/effects/particle"},
            {"water_top", "graphics/level/water/top"},
            {"cloud_small", "graphics/level/clouds/small"}
        };

        return lookup;
    }
};