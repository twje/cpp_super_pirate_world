#pragma once

// Includes
//------------------------------------------------------------------------------
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
    }

    void UnloadGlobalAssets()
    {
        ResourceLocator& locator = ResourceLocator::GetInstance();

        for (auto& [_, filepath] : FONT_MAP)
        {
            locator.GetFontManager().ReleaseResource(filepath);
        }
    }

private:
    std::unordered_map<std::string, TextureDirMapManager> mTextureDirMaps;
};