#include "Settings.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <algorithm>
#include <vector>
#include <unordered_map>

//------------------------------------------------------------------------------
std::unordered_map<FontId, std::string> FONT_MAP = 
{
    { FontId::DEBUG_FONT, "graphics/ui/runescape_uf.ttf" }
};

//------------------------------------------------------------------------------
std::unordered_map<std::string, uint32_t> DEPTHS =
{
    { "bg", 0 },
    { "cloud", 1 },
    { "bg tiles", 2 },
    { "path", 3 },
    { "bg details", 4 },
    { "main", 5 },
    { "water", 6 },
    { "fg", 7 }
};

//------------------------------------------------------------------------------
const std::vector<uint32_t>& GetSortedDepths() 
{    
    static std::vector<uint32_t> sortedDepths;
 
    if (sortedDepths.empty()) 
    {    
        for (const auto& pair : DEPTHS) 
        {
            sortedDepths.push_back(pair.second);
        }        
        std::sort(sortedDepths.begin(), sortedDepths.end());
    }

    return sortedDepths;
}