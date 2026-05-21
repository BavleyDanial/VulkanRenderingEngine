#include <LayerStack.h>

namespace VKRE {

    LayersStack::~LayersStack() {
        for (auto& layer : mLayers)
            layer->OnDetach();
    }

    void LayersStack::PushLayer(std::unique_ptr<Layer> layer) {
        layer->OnAttach();
        mLayers.emplace(mLayers.begin() + mLayerInsertIndex, std::move(layer));
        mLayerInsertIndex++;
    }

    void LayersStack::PushOverlay(std::unique_ptr<Layer> overlay) {
        overlay->OnAttach();
        mLayers.emplace_back(std::move(overlay));
    }
    void LayersStack::PopLayer(std::unique_ptr<Layer> layer) {
        auto it = std::find(mLayers.begin(), mLayers.begin() + mLayerInsertIndex, layer);
        if (it == mLayers.begin() + mLayerInsertIndex) return;

        layer->OnDetach();
        mLayers.erase(it);
        mLayerInsertIndex--;
    }

    void LayersStack::PopOverlay(std::unique_ptr<Layer> overlay) {
        auto it = std::find(mLayers.begin() + mLayerInsertIndex, mLayers.end(), overlay);
        if (it == mLayers.end()) return;

        overlay->OnDetach();
        mLayers.erase(it);
    }
}
