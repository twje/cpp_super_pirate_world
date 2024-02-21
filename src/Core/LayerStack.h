// Includes
//------------------------------------------------------------------------------
// Core
#include "Layer.h"

// System
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
class LayerStack
{
public:
    void PushLayer(std::unique_ptr<Layer> layer)
    {
        if (layer)
        {
            layer->OnEnter();
            mLayers.push_back(std::move(layer));
        }
    }

    void PopLayer()
    {
        if (!mLayers.empty())
        {
            mLayers.back()->OnExit();
            mLayers.pop_back();
        }
    }

    Layer* GetTop() { return mLayers.back().get(); }

    void Clear()
    {
        for (auto& layer : mLayers)
        {
            layer->OnExit();
        }
        mLayers.clear();
    }

    void HandleEvent(const sf::Event& event)
    {
        for (size_t i = mLayers.size(); i-- > 0; )
        {
            if (!mLayers[i]->HandleEvent(event))
            {
                break;
            }
        }
    }

    void Update(const sf::Time& timeslice)
    {
        for (size_t i = mLayers.size(); i-- > 0; )
        {
            if (!mLayers[i]->Update(timeslice))
            {
                break;
            }
        }
    }

    void Draw(sf::RenderWindow& window)
    {
        for (size_t i = 0; i < mLayers.size(); ++i)
        {
            if (!mLayers[i]->Draw(window)) {
                break;
            }
        }
    }

    void Resize(const sf::Vector2f& size) {
        for (size_t i = 0; i < mLayers.size(); ++i)
        {
            mLayers[i]->Resize(size);
        }
    }

private:
    std::vector<std::unique_ptr<Layer>> mLayers;
};