#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

// System
#include <unordered_map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Type aliases
//------------------------------------------------------------------------------
using TextureVector = std::vector<std::unique_ptr<sf::Texture>>;
using TextureMap = std::unordered_map<std::string, TextureVector>;

//------------------------------------------------------------------------------
template<typename Derived, typename T>
class ResourceManager
{
public:
    T* GetResource(std::string filepath)
    {
        auto resource = Find(filepath);
        if (resource)
        {
            return resource->first->get();
        }
        return nullptr;
    }

    bool RequireResource(std::string filepath)
    {
        auto resource = Find(filepath);
        if (resource)
        {
            resource->second++;
            return true;
        }

        std::unique_ptr<T> loadedResource = Load(filepath);
        if (!loadedResource) { return false; }

        auto [inserted, success] = mResources.emplace(filepath, std::make_pair(std::move(loadedResource), 1));
        return success;
    }    

    bool ReleaseResource(std::string filepath)
    {
        auto resource = Find(filepath);
        if (!resource) { return false; }
        --resource->second;
        if (resource->second == 0) { Unload(filepath); }
        return true;
    }    

    void PurgeResources()
    {
        mResources.clear();
    }    

protected:
    fs::path GetResourcePath() const
    {
        return RESOURCES_PATH;
    }

private:
    std::unique_ptr<T> Load(std::string filepath)
    {
        return static_cast<Derived*>(this)->Load(filepath);
    }
    
    bool Unload(std::string filepath)
    {
        auto itr = mResources.find(filepath);
        if (itr == mResources.end())
        {
            return false;
        }
        mResources.erase(itr);
        return true;
    }    

    std::pair<std::unique_ptr<T>, uint32_t>* Find(std::string filepath)
    {
        auto itr = mResources.find(filepath);
        if (itr != mResources.end())
        {
            return &itr->second;
        }
        return nullptr;
    }

    std::unordered_map<std::string, std::pair<std::unique_ptr<T>, uint32_t>> mResources;
};

//------------------------------------------------------------------------------
template <typename T>
class CommonResourceManager : public ResourceManager<CommonResourceManager<T>, T>
{
public:
    std::unique_ptr<T> Load(std::string filepath)
    {
        auto resource = std::make_unique<T>();
        if (!resource->loadFromFile(GetResourcePath() / filepath))
        {
            throw std::runtime_error("Failed to load resource: " + filepath);
        }
        return resource;
    }
};

//------------------------------------------------------------------------------
using TextureManager = CommonResourceManager<sf::Texture>;
using FontManager = CommonResourceManager<sf::Font>;
using SoundBufferManager = CommonResourceManager<sf::SoundBuffer>;

//------------------------------------------------------------------------------
class MusicManager : public ResourceManager<MusicManager, sf::Music>
{
public:
    std::unique_ptr<sf::Music> Load(std::string filepath)
    {
        auto musicPtr = std::make_unique<sf::Music>();
        if (!musicPtr->openFromFile(GetResourcePath() / filepath))
        {
            throw std::runtime_error("Failed to load music: " + filepath);
        }
        return musicPtr;
    }
};

//------------------------------------------------------------------------------
class TextureVectorManager : public ResourceManager<TextureVectorManager, TextureVector>
{
public:
    std::unique_ptr<TextureVector> Load(std::string directory)
    {
        // Loads all textures from a directory into a vector.

        std::unique_ptr<TextureVector> textureVectorPtr = std::make_unique<TextureVector>();

        for (const auto& filepath : fs::directory_iterator(GetResourcePath() / directory))
        {            
            fs::path relativePath = fs::relative(filepath.path(), GetResourcePath());            
            
            auto texture = std::make_unique<sf::Texture>();
            if (!texture->loadFromFile(GetResourcePath() / relativePath))
            {
                throw std::runtime_error("Failed to load resource: " + relativePath.string());
            }
            textureVectorPtr->push_back(std::move(texture));
        }
        return textureVectorPtr;
    }
};

//------------------------------------------------------------------------------
class TextureMapManager : public ResourceManager<TextureMapManager, TextureMap>
{
public:
    std::unique_ptr<TextureMap> Load(std::string directory)
    {
        return nullptr; // implement
    }
};

//------------------------------------------------------------------------------
class TextureDirMapManager : public ResourceManager<TextureDirMapManager, TextureMap>
{
public:
    std::unique_ptr<TextureMap> Load(std::string directory)
    {
        // Loads and organizes textures by subdirectory into a map.

        std::unique_ptr<TextureMap> textureMapPtr = std::make_unique<TextureMap>();
        for (const auto& subDirectory : fs::directory_iterator(GetResourcePath() / directory))
        {
            if (fs::is_directory(subDirectory.status()))
            {
                TextureVector textures;
                for (const auto& filepath : fs::directory_iterator(subDirectory.path()))
                {
                    fs::path relativePath = fs::relative(filepath.path(), GetResourcePath());

                    auto texture = std::make_unique<sf::Texture>();
                    if (!texture->loadFromFile(GetResourcePath() / relativePath))
                    {
                        throw std::runtime_error("Failed to load resource: " + relativePath.string());
                    }
                    textures.push_back(std::move(texture));
                }

                std::string directoryName = subDirectory.path().filename().string();
                textureMapPtr->emplace(directoryName, std::move(textures));
            }
        }
        return textureMapPtr;
    }
};

//------------------------------------------------------------------------------
class ResourceLocator
{
public:
    static ResourceLocator& GetInstance()
    {
        static ResourceLocator instance;
        return instance;
    }

    // Basic managers
    TextureManager& GetTextureManager() { return mTextureManager; }
    FontManager& GetFontManager() { return mFontManager; }
    SoundBufferManager& GetSoundBufferManager() { return mSoundBufferManager; }
    MusicManager& GetMusicManager() { return mMusicManager; }

    // Specialized managers
    TextureVectorManager& GetTextureVectorManager() { return mTextureVectorManager; }
    TextureMapManager& GetTextureMapManager() { return mTextureMapManager; }
    TextureDirMapManager& GetTextureDirMapManager() { return mTextureDirMapManager; }

private:
    ResourceLocator() = default;
    ResourceLocator(const ResourceLocator&) = delete;
    ResourceLocator& operator=(const ResourceLocator&) = delete;

    // Basic managers
    TextureManager mTextureManager;
    FontManager mFontManager;
    SoundBufferManager mSoundBufferManager;
    MusicManager mMusicManager;

    // Specialized managers
    TextureVectorManager mTextureVectorManager;
    TextureMapManager mTextureMapManager;
    TextureDirMapManager mTextureDirMapManager;
};