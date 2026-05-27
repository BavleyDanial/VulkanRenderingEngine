#include <Scene/Scene.h>

#include <Scene/Components.h>

namespace VKRE {

    Scene::Scene() {
        mWorld.component<TransformComponent>();
        mWorld.component<StaticMeshComponent>();
    }

    Entity Scene::AddEntity(std::string_view name) {
        flecs::entity e = mWorld.entity(name.data());
        e.set<TransformComponent>({});

        return Entity(e);
    }

    Entity Scene::AddChildEntity(Entity parent, std::string_view name) {
        assert(parent.IsValid() && "Tried to create a child entity to an invalid parent");

        Entity e = AddEntity(name);
        e.SetParent(parent);

        return e;
    }

    void Scene::DestroyEntity(Entity entity) {
        entity.Destroy();
    }

    void Scene::OnUpdate(float dt) {
        mWorld.progress(dt);
    }

}
