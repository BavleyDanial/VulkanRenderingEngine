#pragma once

#include "ResourceHandles.h"
#include "ResourceManager.h"

namespace VKRE {

    template<typename Tag>
    class ResourceRef {
    public:
        ResourceRef() = default;
        ResourceRef(ResourceHandle<Tag> handle, ResourceManager* manager)
            :mHandle(handle), mManager(manager) { AddRef(); }

        ~ResourceRef() { Release(); }

        ResourceRef(const ResourceRef& other)
            :mHandle(other.mHandle), mManager(other.mManager) { AddRef(); }

        ResourceRef& operator=(const ResourceRef& other) {
            if (this != &other) {
                Release();
                mHandle = other.mHandle;
                mManager = other.mManager;
                AddRef();
            }

            return *this;
        }

        ResourceRef(ResourceRef&& other) noexcept
            :mHandle(other.mHandle), mManager(other.mManager) {
            other.mHandle = ResourceHandle<Tag>::Null();
            other.mManager = nullptr;
        }

        ResourceRef& operator=(ResourceRef&& other) noexcept {
            if (this != &other) {
                Release();
                mHandle = other.mHandle;
                mManager = other.mManager;
                other.mHandle = ResourceHandle<Tag>::Null();
                other.mManager = nullptr;
            }

            return *this;
        }

        ResourceHandle<Tag> Get() const { return mHandle; }
        bool IsValid() const { return mHandle.IsValid(); }
        explicit operator bool() const { return IsValid(); }
        explicit operator ResourceHandle<Tag>() const { return mHandle; }

    private:
        void AddRef() {
            if (mHandle.IsValid() && mManager)
                mManager->AddRef(mHandle);
        }

        void Release() {
            if (mHandle.IsValid() && mManager)
                mManager->DestroyRef(mHandle);
            mHandle = ResourceHandle<Tag>::Null();
            mManager = nullptr;
        }

    private:
        ResourceHandle<Tag> mHandle = ResourceHandle<Tag>::Null();
        ResourceManager* mManager = nullptr;
    };


}
