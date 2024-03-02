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
public:
    void LoadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().RequireResource(filepath);
        }

        // Textures organised by sub-directories
        locator.GetTextureDirMapManager().RequireResource("graphics/level/palms");

        mTextureDirMaps["palms"] = locator.GetTextureDirMapManager().GetResource("graphics/level/palms");
    }

    void UnloadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().ReleaseResource(filepath);
        }

        // Textures organised by sub-directories
        locator.GetTextureDirMapManager().RequireResource("graphics/level/palms");
    }

    TextureMap& GetTextureDirMap(const std::string& id) const { return *mTextureDirMaps.at(id); }

private:
    std::unordered_map<std::string, TextureMap*> mTextureDirMaps;
};