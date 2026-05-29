#pragma once

#include <flecs.h>

#include "Entity.h"

#include <string_view>

namespace VKRE {

    class Scene {
    public:
        Scene();
        ~Scene() = default;

        Entity AddEntity(std::string_view name = "");
        void DestroyEntity(Entity entity);

        Entity AddCamera(std::string_view name, float fov = 60.0f, float near = 0.1f, float far = 1000.0f);
        Entity AddChildEntity(Entity parent, std::string_view name = "");

        void OnUpdate(float dt);

        flecs::world& GetFlecsWorld() { return mWorld; }
        const flecs::world& GetFlecsWorld() const { return mWorld; }

    private:
        flecs::world mWorld;
    };

}
