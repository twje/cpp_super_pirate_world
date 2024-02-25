#include "Resources.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <unordered_map>
#include <iostream>
#include <stdexcept>

//------------------------------------------------------------------------------
ResourceManager& ResourceManager::GetInstance() 
{
    static ResourceManager instance;
    return instance;
}

//------------------------------------------------------------------------------
TextureVectorPtr ResourceManager::LoadTexuresFromDirectory(const fs::path& directory)
{
    TextureVectorPtr textures = std::make_unique<TextureVector>();

    for (const auto& filepath : fs::directory_iterator(GetResourcePath() / directory))
    {        
        fs::path relativePath = fs::relative(filepath.path(), GetResourcePath());
        textures->push_back(&LoadTexture(relativePath));
    }
    return textures;
}

//------------------------------------------------------------------------------
TextureMapPtr ResourceManager::LoadTexuresFromSubDirectory(const fs::path& directory)
{    
    TextureMapPtr textureMap = std::make_unique<std::unordered_map<std::string, TextureVector>>();
    for (const auto& subDirectory : fs::directory_iterator(GetResourcePath() / directory))
    {
        if (fs::is_directory(subDirectory.status()))
        {
            std::vector<sf::Texture*> textures;            
            for (const auto& filepath : fs::directory_iterator(subDirectory.path()))
            {     
                fs::path relativePath = fs::relative(filepath.path(), GetResourcePath());
                textures.push_back(&LoadTexture(relativePath.generic_string()));
            }            
            (*textureMap)[subDirectory.path().filename().string()] = std::move(textures);
        }
    }
    return textureMap;
}

//------------------------------------------------------------------------------
sf::Texture& ResourceManager::LoadTexture(const fs::path& filename)
{
    return LoadResource<sf::Texture>(mTextureStore, filename);
}

//------------------------------------------------------------------------------
sf::Font& ResourceManager::LoadFont(const fs::path& filename)
{    
    return LoadResource<sf::Font>(mFontStore, filename);
}

//------------------------------------------------------------------------------
sf::SoundBuffer& ResourceManager::LoadSoundBuffer(const fs::path& filename)
{    
    return LoadResource<sf::SoundBuffer>(mSoundBufferStore, filename);
}

//------------------------------------------------------------------------------
sf::Music& ResourceManager::LoadMusic(const fs::path& filename)
{    
    auto it = mMusicStore.find(filename.string());
    if (it != mMusicStore.end())
    {
        return *(it->second);
    }

    auto musicPtr = std::make_unique<sf::Music>();
    if (!musicPtr->openFromFile(GetResourcePath() / filename))
    {
        throw std::runtime_error("Failed to load music: " + filename.string());
    }

    sf::Music& musicRef = *musicPtr;
    mMusicStore.emplace(filename.string(), std::move(musicPtr));
    return musicRef;    
}