#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "Interfaces.h"
#include "LevelMap.h"
#include "GameData.h"

//------------------------------------------------------------------------------
class Level
{
public:
    Level(LevelMap& levelMap, GameData& gameData, IGame& gameCallbacks)
        : mLevelMap(levelMap)
        , mGameData(gameData)    
        , mGameCallbacks(gameCallbacks)
    { }

    void Draw(sf::RenderWindow& window)
    {
        mLevelMap.Draw(window);
    }

private:
    LevelMap& mLevelMap;
    GameData& mGameData;
    IGame& mGameCallbacks;
};