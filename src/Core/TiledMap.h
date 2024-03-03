#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>
#include <tileson.hpp>

// Core
#include "Core/ResourceManager.h"
#include "Core/CustomExceptions.h"

// System 
#include <filesystem>



namespace fs = std::filesystem;

//------------------------------------------------------------------------------
template<typename T>
sf::Rect<T> ConvertToSFMLRect(const tson::Rect& rect)
{
    return sf::Rect<T>({ static_cast<T>(rect.x), static_cast<T>(rect.y) },
                       { static_cast<T>(rect.width), static_cast<T>(rect.height) });
}

//------------------------------------------------------------------------------
template<typename T>
sf::Vector2f ConvertToSFMLVector2f(const tson::Vector2<T>& tVector)
{
    return sf::Vector2f(static_cast<float>(tVector.x), static_cast<float>(tVector.y));
}

//------------------------------------------------------------------------------
enum class TiledMapTilesetType
{
    Unknown = 0,
    ImageTileset = 1,
    ImageCollectionTileset = 2,
};

//------------------------------------------------------------------------------
enum class TiledMapLayerType
{
    Unknown = 0,
    TileLayer = 1,
    ObjectGroup = 2,
};

//------------------------------------------------------------------------------
enum class TiledMapObjectType
{
    Unknown = 0,
    Object = 1
};

//------------------------------------------------------------------------------
class TiledMapObject
{
public:
    TiledMapObject(tson::Object& object, tson::Layer& layer)
        : mType(TiledMapObjectType::Unknown)
        , mGid(0)        
        , mScale(1.0f, 1.0f)
        , mSize(ConvertToSFMLVector2f(object.getSize()))
        , mName(object.getName())
        , mPropertyCollection(object.getProperties())
    {
        mPosition = ConvertToSFMLVector2f(object.getPosition());
        mPosition.x = std::round(mPosition.x);
        mPosition.y = std::round(mPosition.y);

        if (object.getObjectType() == tson::ObjectType::Object)
        {
            mType = TiledMapObjectType::Object;
            mGid = object.getGid();
            ComputeTextureRegion(layer);
            
            mPosition.y -= object.getSize().y;  // Account for y origin of tile object
        }
        
        // Do not support composite flips
        if (object.hasFlipFlags(tson::TileFlipFlags::Vertically))
        {
            mScale.y = -1.0f;
            mPosition.y += object.getSize().y;
        }
        else if (object.hasFlipFlags(tson::TileFlipFlags::Horizontally))
        {
            throw NotImplementedException();
        }
    }

    TiledMapObjectType GetType() const { return mType; }
    uint32_t GetGid() const { return mGid; }
    const sf::IntRect& GetTextureRegion() const { return mTextureRegion; }
    const sf::Vector2f& GetPosition() const { return mPosition; }    
    const sf::Vector2f& GetScale() const { return mScale; }
    const sf::Vector2f& GetSize() const { return mSize; }
    const std::string& GetName() const { return mName; }

    template<typename T>
    T GetPropertyValue(const std::string& name) const
    {
        //Remove const to call method
        return const_cast<TiledMapObject*>(this)->mPropertyCollection.getValue<T>(name);
    }

private:
    void ComputeTextureRegion(tson::Layer& layer)
    {
        tson::Tileset* tileset = layer.getMap()->getTilesetByGid(mGid);
        uint32_t id = mGid - tileset->getFirstgid() + 1;
        tson::Tile* tile = tileset->getTile(id);
        mTextureRegion = sf::IntRect(ConvertToSFMLRect<float>(tile->getDrawingRect()));        
    }

    TiledMapObjectType mType;
    uint32_t mGid;
    sf::IntRect mTextureRegion;
    sf::Vector2f mPosition;
    sf::Vector2f mScale;
    sf::Vector2f mSize;
    std::string mName;
    tson::PropertyCollection mPropertyCollection;
};

//------------------------------------------------------------------------------
class TiledMapTile
{
public:
    TiledMapTile(tson::Tile& tile, tson::Tileset& tileset, fs::path mapFilepath)
        : mId(tile.getId() - 1)
        , mGid(tile.getGid())
        , mTextureRegion(ConvertToSFMLRect<float>(tile.getDrawingRect()))
        , mScale(1.0f, 1.0f)
    { 
        if (tileset.getType() == tson::TilesetType::ImageCollectionTileset)
        {
            fs::path textureFullpath = fs::absolute(mapFilepath.parent_path() / tile.getImage());
            assert(fs::exists(textureFullpath));
            mTextureRelativeFilepath = fs::relative(textureFullpath, RESOURCES_PATH);
        }

        // Do not support composite flips
        if (tile.hasFlipFlags(tson::TileFlipFlags::Vertically))
        {
            throw NotImplementedException();
        }
        else if (tile.hasFlipFlags(tson::TileFlipFlags::Horizontally))
        {
            throw NotImplementedException();
        }
    }

    uint32_t GetId() const { return mId; }
    uint32_t GetGid() const { return mGid; }
    const sf::IntRect& GetTextureRegion() const { return mTextureRegion; }    
    fs::path GetTextureRelativeFilepath() const { return mTextureRelativeFilepath; }    
    const sf::Vector2f& GetScale() const { return mScale; }

private:
    uint32_t mId;
    uint32_t mGid;
    sf::IntRect mTextureRegion;    
    fs::path mTextureRelativeFilepath;
    sf::Vector2f mScale;
};

//------------------------------------------------------------------------------
class TiledMapTileset
{
public:
    TiledMapTileset(tson::Tileset& tileset, fs::path mapFilepath)
        : mType(TiledMapTilesetType::Unknown)
    {   
        for (tson::Tile& tile : tileset.getTiles())
        {
            mTiles.emplace_back(tile, tileset, mapFilepath);
        }        

        if (tileset.getType() == tson::TilesetType::ImageTileset)
        {
            fs::path textureFullpath = fs::absolute(mapFilepath.parent_path() / tileset.getFullImagePath());
            assert(fs::exists(textureFullpath));
            mTextureRelativeFilepath = fs::relative(textureFullpath, RESOURCES_PATH);
        }

        if (tileset.getType() == tson::TilesetType::ImageTileset)
        {
            mType = TiledMapTilesetType::ImageTileset;
        }
        else if (tileset.getType() == tson::TilesetType::ImageCollectionTileset)
        {
            mType = TiledMapTilesetType::ImageCollectionTileset;
        }
    }

    std::vector<TiledMapTile>& GetTiles() { return mTiles; }

    void LoadTextures(std::unordered_map<uint32_t, sf::Texture*>& textureLookup)
    {
        TextureManager& textureManager = ResourceLocator::GetInstance().GetTextureManager();
        
        if (mType == TiledMapTilesetType::ImageTileset)
        {
            if (!textureManager.RequireResource(mTextureRelativeFilepath.string()))
            {
                assert(false);
            }
            
            for (TiledMapTile& tile : mTiles)
            {
                sf::Texture* texture = textureManager.GetResource(mTextureRelativeFilepath.string());
                textureLookup[tile.GetGid()] = texture;
            }
        }
        else if (mType == TiledMapTilesetType::ImageCollectionTileset)
        {
            for (TiledMapTile& tile : mTiles)
            {                       
                if (!textureManager.RequireResource(tile.GetTextureRelativeFilepath().string()))
                {
                    assert(false);
                }
             
                sf::Texture* texture = textureManager.GetResource(tile.GetTextureRelativeFilepath().string());
                textureLookup[tile.GetGid()] = texture;                
            }
        }
    }

    void UnloadTextures()
    {
        TextureManager& textureManager = ResourceLocator::GetInstance().GetTextureManager();

        if (mType == TiledMapTilesetType::ImageTileset)
        {
            textureManager.ReleaseResource(mTextureRelativeFilepath.string());            
        }
        else if (mType == TiledMapTilesetType::ImageCollectionTileset)
        {
            for (TiledMapTile& tile : mTiles)
            {
                textureManager.RequireResource(tile.GetTextureRelativeFilepath().string());                                
            }
        }
    }

private:
    std::vector<TiledMapTile> mTiles;
    fs::path mTextureRelativeFilepath;
    TiledMapTilesetType mType;
};

//------------------------------------------------------------------------------
class TiledMapLayer
{
public:
    TiledMapLayer(tson::Layer& layer, std::unordered_map<uint32_t, TiledMapTile*> tileMap)
        : mTileCount(ConvertToSFMLVector2f(layer.getSize()))
        , mType(TiledMapLayerType::Unknown)
        , mName(layer.getName())
    {
        if (layer.getType() == tson::LayerType::TileLayer)
        {      
            for (auto& [index, tile] : layer.getTileData())
            {
                mTileData[index] = tileMap.at(tile->getGid());
            }
        }
        else if (layer.getType() == tson::LayerType::ObjectGroup)
        {
            for (tson::Object& object : layer.getObjects())
            {
                mObjects.emplace_back(object, layer);
            }
        }

        if (layer.getType() == tson::LayerType::TileLayer)
        {
            mType = TiledMapLayerType::TileLayer;
        }
        else if (layer.getType() == tson::LayerType::ObjectGroup)
        {
            mType = TiledMapLayerType::ObjectGroup;
        }
    }

    TiledMapTile* GetTile(uint32_t x, uint32_t y) const
    { 
        if (mTileData.count({ x, y }) > 0)
        {
            return mTileData.at({ x, y });
        }
        return nullptr;
    }

    const std::vector<TiledMapObject>& GetObjects() const { return mObjects; }
    const sf::Vector2u& GetTileCount() const { return mTileCount; }
    TiledMapLayerType GetType() const { return mType; }
    const std::string& GetName() const { return mName; }

private:
    std::set<uint32_t> mUniqueFlaggedTiles;
    std::vector<TiledMapObject> mObjects;
    std::map<std::tuple<int32_t, int32_t>, TiledMapTile*> mTileData;
    sf::Vector2u mTileCount;
    TiledMapLayerType mType;
    std::string mName;
};

//------------------------------------------------------------------------------
class TiledMap
{
public:
    TiledMap(fs::path mapFilepath)
    {
        if (!mapFilepath.is_absolute())
        {
            mapFilepath = RESOURCES_PATH / mapFilepath;
        }
        mapFilepath = fs::canonical(mapFilepath);
        
        tson::Tileson parser;
        auto sourceData = parser.parse(mapFilepath.string());

        for (tson::Tileset& tileset : sourceData->getTilesets())
        {
            mTilesets.emplace_back(tileset, mapFilepath);

            for (TiledMapTile& tile : mTilesets.back().GetTiles())
            {                
                mTileMap.emplace(tile.GetGid(), &tile);
            }
        }

        for (tson::Layer& layer : sourceData->getLayers())
        {
            mLayers.emplace_back(layer, mTileMap);
        }

        mTileSize = ConvertToSFMLVector2f(sourceData->getTileSize());

        sourceData.reset();
    }

    void LoadTextures()
    {
        for (TiledMapTileset& tileset : mTilesets)
        {
            tileset.LoadTextures(mTextureLookup);
        }
    }

    void UnloadTextures()
    {
        for (TiledMapTileset& tileset : mTilesets)
        {
            tileset.UnloadTextures();            
        }
    }
       
    sf::Texture* GetTetxure(uint32_t gid) const { return mTextureLookup.at(gid); }
    const std::vector<TiledMapLayer>& GetLayers() const { return mLayers; }    
    const sf::Vector2f GetTileSize() const { return mTileSize; }

private:
    std::unordered_map<uint32_t, sf::Texture*> mTextureLookup;
    std::unordered_map<uint32_t, TiledMapTile*> mTileMap;
    std::vector<TiledMapTileset> mTilesets;
    std::vector<TiledMapLayer> mLayers;
    sf::Vector2f mTileSize;
};

//------------------------------------------------------------------------------
class TiledMapRenderer
{
public:
    TiledMapRenderer(TiledMap& tiledMap)
        : mTiledMap(tiledMap)
    { 
        mTiledMap.LoadTextures();
    }
    
    ~TiledMapRenderer()
    {
        mTiledMap.UnloadTextures();
    }

    void Draw(sf::RenderTarget& window)
    {
        for (const TiledMapLayer& layer : mTiledMap.GetLayers())
        {
            Draw(window, layer);
        }
    }

    void Draw(sf::RenderTarget& window, uint32_t index)
    {
        Draw(window, mTiledMap.GetLayers().at(index));
    }

    void Draw(sf::RenderTarget& window, const TiledMapLayer& layer)
    {
        if (layer.GetType() == TiledMapLayerType::TileLayer)
        {
            DrawTileLayer(window, layer);
        }
        else if (layer.GetType() == TiledMapLayerType::ObjectGroup)
        {
            DrawObjectGroup(window, layer);
        }
    }
    
private:
    void DrawTileLayer(sf::RenderTarget& window, const TiledMapLayer& layer)
    {
        sf::Vector2f tileSize = mTiledMap.GetTileSize();

        for (uint32_t y = 0; y < layer.GetTileCount().y; y++)
        {
            for (uint32_t x = 0; x < layer.GetTileCount().x; x++)
            {
                const TiledMapTile* tile = layer.GetTile(x, y);

                if (tile != nullptr)
                {
                    sf::Texture* texture = mTiledMap.GetTetxure(tile->GetGid());
                    sf::Sprite sprite(*texture);
                    sprite.setTextureRect(tile->GetTextureRegion());
                    sprite.setPosition({ x * tileSize.x, y * tileSize.y });
                    sprite.setScale(tile->GetScale());

                    window.draw(sprite);
                }
            }
        }
    }

    void DrawObjectGroup(sf::RenderTarget& window, const TiledMapLayer& layer)
    {
        for (const TiledMapObject& object : layer.GetObjects())
        {
            if (object.GetType() == TiledMapObjectType::Object)
            {
                sf::Texture* texture = mTiledMap.GetTetxure(object.GetGid());
                sf::Sprite sprite(*texture);
                sprite.setTextureRect(object.GetTextureRegion());
                sprite.setPosition(object.GetPosition());
                sprite.setScale(object.GetScale());

                window.draw(sprite);
            }
        }
    }

    TiledMap& mTiledMap;
};