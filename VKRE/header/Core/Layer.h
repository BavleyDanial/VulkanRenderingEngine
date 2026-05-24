#pragma once

#include <string_view>
#include <string>

namespace VKRE {

    class Layer {
    public:
        Layer(const std::string& debugName)
            :mDebugName(debugName) {}

        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnUIRender() {}

        const std::string_view GetName() const { return mDebugName; }

    protected:
        std::string mDebugName = "";
    };

}
