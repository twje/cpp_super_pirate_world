#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SFML/Graphics.hpp>

// Forward declarations
//------------------------------------------------------------------------------
class LayerStack;

//------------------------------------------------------------------------------
class Layer
{
public:
    virtual ~Layer() = default;
    Layer(LayerStack& layerStack)
        : mLayerStack(layerStack)
    { }

    // Hooks
    virtual bool HandleEvent(const sf::Event& event) { return true; };
    virtual bool Update(const sf::Time& timeslice) { return true; };
    virtual bool Draw(sf::RenderWindow& window) { return true; };
    virtual void Resize(const sf::Vector2f& size) { };
    virtual void OnEnter() { };
    virtual void OnExit() { };

    // Layer management    
    LayerStack& GetLayerStack() { return mLayerStack; }

private:
    LayerStack& mLayerStack;
};
