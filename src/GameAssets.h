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

        // Individual textures
        for (auto& [_, filepath] : GetTextureLookup())
        {
            locator.GetTextureManager().RequireResource(filepath);
        }

        // Textures organised by sub-directories        
        for (auto& [_, filepath] : GetTextureDirMapsLookup())
        {
            locator.GetTextureDirMapManager().RequireResource(filepath);
        }

        // Textures in directory
        for (auto& [_, filepath] : GetTextureVecLookup())
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

        // Individual textures
        for (auto& [_, filepath] : GetTextureLookup())
        {
            locator.GetTextureManager().ReleaseResource(filepath);
        }

        // Textures organised by sub-directories
        for (auto& [_, filepath] : GetTextureDirMapsLookup())
        {
            locator.GetTextureDirMapManager().ReleaseResource(filepath);
        }        

        // Textures in directory
        for (auto& [_, filepath] : GetTextureVecLookup())
        {
            locator.GetTextureVectorManager().ReleaseResource(filepath);
        }
    }

    sf::Texture& GetTexture(const std::string& id) const
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();
        AssetLookup& lookup = GetTextureLookup();
        assert(lookup.count(id) > 0);
        
        return *locator.GetTextureManager().GetResource(lookup.at(id));
    }

    TextureMap& GetTextureDirMap(const std::string& id) const 
    { 
        ResourceLocator& locator = ResourceLocator::GetInstance();
        AssetLookup& lookup = GetTextureDirMapsLookup();
        assert(lookup.count(id) > 0);

        return *locator.GetTextureDirMapManager().GetResource(lookup.at(id));
    }
    
    TextureVector& GetTextureVec(const std::string& id) const 
    { 
        ResourceLocator& locator = ResourceLocator::GetInstance();
        AssetLookup& lookup = GetTextureVecLookup();
        assert(lookup.count(id) > 0);

        return *locator.GetTextureVectorManager().GetResource(lookup.at(id));
    }

private:
    static AssetLookup& GetTextureLookup()
    {
        static AssetLookup lookup = {
            {"saw_chain", "graphics/enemies/saw/saw_chain.png"},
            {"spike", "graphics/enemies/spike_ball/Spiked Ball.png"},
            {"spike_chain", "graphics/enemies/spike_ball/spiked_chain.png"},
            {"pearl", "graphics/enemies/bullets/pearl.png"},
            {"water_body", "graphics/level/water/body.png"},
            {"cloud_large", "graphics/level/clouds/large_cloud.png"},
        };

        return lookup;
    }

    static AssetLookup& GetTextureDirMapsLookup()
    {
        static AssetLookup lookup = { 
            {"palms", "graphics/level/palms"},
            {"player", "graphics/player"},
            {"shell", "graphics/enemies/shell"},
            {"items", "graphics/items"}
        };
        
        return lookup;
    }

    static AssetLookup& GetTextureVecLookup()
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