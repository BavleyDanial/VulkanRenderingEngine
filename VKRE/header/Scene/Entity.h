#pragma once

#include <flecs.h>
#include <cassert>

namespace VKRE {

    class Entity {
    private:
        flecs::entity mHandle;

    public:
        Entity(flecs::entity handle)
            :mHandle(handle) {}

        Entity() = default;
        ~Entity() = default;

        template<typename T>
        Entity& Add(T&& component) {
            assert(mHandle.is_valid() && "Attempted to add component to invalid entity");
            mHandle.set<T>(std::forward<T>(component));
            return *this;
        }

        Entity& SetParent(Entity parent) {
            assert(mHandle.is_valid() && "Attempted to add parent to invalid entity");
            assert(parent.IsValid() && "Attempted to add invalid parent to entity");
            mHandle.child_of(parent.mHandle);
            return *this;
        }

        template<typename T>
        Entity& Remove() {
            assert(mHandle.is_valid() && "Attempted to remove component to invalid entity");
            mHandle.remove<T>();
            return *this;
        }

        template<typename T>
        const T& Get() {
            assert(mHandle.is_valid() && "Attempted to get const component from invalid entity");
            return mHandle.get<T>();
        }

        template<typename T>
        T& GetMutable() {
            assert(mHandle.is_valid() && "Attempted to get mutable component from invalid entity");
            return mHandle.get_mut<T>();
        }

        template<typename T>
        bool Has() {
            assert(mHandle.is_valid() && "Attempted to query component existance from invalid entity");
            return mHandle.has<T>();
        }

        void Destroy() {
            if (mHandle.is_valid())
                mHandle.destruct();
        }

        bool IsValid() const { return mHandle.is_valid(); }

        bool operator==(const Entity& other) const { return mHandle == other.mHandle; }
        bool operator!=(const Entity& other) const { return mHandle != other.mHandle; }
    };

}
