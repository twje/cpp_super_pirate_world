#pragma once

//------------------------------------------------------------------------------
class IGame
{
public:
    virtual void SwitchLevel() = 0;
};

//------------------------------------------------------------------------------
class ILevel
{
public:
    virtual void CreatePearl() = 0;
};