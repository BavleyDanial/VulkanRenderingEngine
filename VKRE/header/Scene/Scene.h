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
        Entity AddChildEntity(Entity parent, std::string_view name = "");
        void DestroyEntity(Entity entity);

        void OnUpdate(float dt);

        flecs::world& GetFlecsWorld() { return mWorld; }
        const flecs::world& GetFlecsWorld() const { return mWorld; }

    private:
        flecs::world mWorld;
    };

}
