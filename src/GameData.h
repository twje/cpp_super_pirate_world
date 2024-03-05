#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <cstdint>

//------------------------------------------------------------------------------
class GameData
{
public:
    GameData()
        : mCoins(0)
        , mHealth(5)
    { }

    void AddCoins(uint32_t coins)
    {
        mCoins += coins;
        if (mCoins >= 100)
        {
            mCoins -= 100;
            mHealth += 1;
        }
        // self.ui.show_coins(self.coins)
    }

    void AddHealth(uint32_t health)
    {
        mHealth += health;
        // self.ui.show_coins(self.coins)
    }

    uint32_t GetCurrentLevel() { return 0; }

private:
    uint32_t mCoins;
    uint32_t mHealth;
};