#include "Player.h"

// Includes
//------------------------------------------------------------------------------
// Game
#include "Sprites.h"

//------------------------------------------------------------------------------
void Player::PlatformMove(const sf::Time& timeslice)
{
    GameObjectManager& manager = GameObjectManager::Instance();
    if (GameObject* object = manager.GetInstance(mPlatformId))
    {
        MovingSprite* platform = static_cast<MovingSprite*>(object);        
        sf::Vector2f delta = platform->GetDirection() * platform->GetSpeed() * timeslice.asSeconds();
        mHitbox.MoveX(delta.x);
        mHitbox.MoveY(delta.y);
    }
}