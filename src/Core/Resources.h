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
using TextureVector = std::vector<sf::Texture*>;
using TextureVectorPtr = std::unique_ptr<TextureVector>;
using TextureMapPtr = std::unique_ptr<std::unordered_map<std::string, TextureVector>>;

//------------------------------------------------------------------------------
class ResourceManager
{
public:
    static ResourceManager& GetInstance();

    // Specialised loader methods
    TextureVectorPtr LoadTexuresFromDirectory(const fs::path& directory);
    TextureMapPtr LoadTexuresFromSubDirectory(const fs::path& directory);

    // Loader methods
    sf::Texture& LoadTexture(const fs::path& filename);
    sf::Font& LoadFont(const fs::path& filename);
    sf::SoundBuffer& LoadSoundBuffer(const fs::path& filename);
    sf::Music& LoadMusic(const fs::path& filename);

private:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Utility methods
    template<typename Resource>
    Resource& LoadResource(std::unordered_map<std::string, Resource>& store, const fs::path& filename)
    {
        auto it = store.find(filename.string());
        if (it != store.end()) {
            return it->second;
        }

        Resource resource;
        if (!resource.loadFromFile(GetResourcePath() / filename))
        {
            throw std::runtime_error("Failed to load resource: " + filename.string());
        }

        auto [inserted, success] = store.emplace(filename.string(), std::move(resource));
        return inserted->second;
    }

    fs::path GetResourcePath() const 
    { 
        return RESOURCES_PATH; 
    }

    std::unordered_map<std::string, sf::Texture> mTextureStore;
    std::unordered_map<std::string, sf::Font> mFontStore;
    std::unordered_map<std::string, sf::SoundBuffer> mSoundBufferStore;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> mMusicStore;
};