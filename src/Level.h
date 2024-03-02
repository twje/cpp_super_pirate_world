#pragma once

// Includes
//------------------------------------------------------------------------------
// Game
#include "LevelMap.h"
#include "GameData.h"

//------------------------------------------------------------------------------
class Level
{
public:
    Level(LevelMap& levelMap, GameData& gameData)
        : mLevelMap(levelMap)
        , mGameData(gameData)    
    { }

    void Draw(sf::RenderWindow& window)
    {
        mLevelMap.Draw(window);
    }

private:
    LevelMap& mLevelMap;
    GameData& mGameData;
};