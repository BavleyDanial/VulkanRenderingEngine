#pragma once

#include "ResourceHandles.h"

#include <vector>
#include <cassert>
#include <concepts>
#include <print>

namespace VKRE {

    template<typename Tag, typename THot, typename TCold>
    class ResourcePool {
    public:
        void Init(uint32_t initCapacity = 64) {
            assert(mCapacity == 0 && "ResourcePool::Init called more than once");
            Reserve(initCapacity);
            mGenerations[0] = 0;
            mOccupied[0] = false;
        }

        ResourceHandle<Tag> Allocate() {
            uint32_t index = 0;

            if (!mFreeList.empty()) {
                index = mFreeList.back();
                mFreeList.pop_back();
            } else {
                index = mNextFreshSlot++;

                if (index >= mCapacity) {
                    Reserve(mCapacity * 2);
                }
            }

            assert(index != 0 && "Slot 0 is reserved and must never be allocated");
            assert(!mOccupied[index] && "Allocated a slot that is already occupied");

            mOccupied[index] = true;
            mLiveCount++;

            if (mGenerations[index] == 0)
                mGenerations[index] = 1; // We have used this slot for the first time

            return ResourceHandle<Tag>{ index, mGenerations[index] };
        }

        void Release(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) {
                std::println("ResourcePool::Release attempted to release a stale or invalid handle (index={}, gen={})", static_cast<uint32_t>(handle.index), static_cast<uint32_t>(handle.generation));
                return;
            }

            mOccupied[handle.index] = false;
            mLiveCount--;

            mGenerations[handle.index]++;
            if (mGenerations[handle.index] == 0)
                mGenerations[handle.index] = 1; // if we wrapped to 0 go back to 1

            mFreeList.push_back(handle.index);
        }

        bool IsValid(ResourceHandle<Tag> handle) const {
            if (!handle.IsValid())                                  return false;
            if (handle.index >= mCapacity)                          return false;
            if (!mOccupied[handle.index])                           return false;
            if (mGenerations[handle.index] != handle.generation)    return false;
            return true;
        }

        THot* GetHot(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) return nullptr;
            return &mHot[handle.index];
        }

        const THot* GetHot(ResourceHandle<Tag> handle) const {
            if (!IsValid(handle)) return nullptr;
            return &mHot[handle.index];
        }

        TCold* GetCold(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) return nullptr;
            return &mCold[handle.index];
        }

        const TCold* GetCold(ResourceHandle<Tag> handle) const {
            if (!IsValid(handle)) return nullptr;
            return &mCold[handle.index];
        }

        template<typename Fn>
        requires std::invocable<Fn, uint32_t, THot&, TCold&>
        void ForEach(Fn&& fn) {
            for (uint32_t i = 1; i < mNextFreshSlot; i++) {
                if (mOccupied[i])
                    fn(i, mHot[i], mCold[i]);
            }
        }

        template<typename Fn>
        requires std::invocable<Fn, uint32_t, THot&, TCold&>
        void ForEach(Fn&& fn) const {
            for (uint32_t i = 1; i < mNextFreshSlot; i++) {
                if (mOccupied[i])
                    fn(i, mHot[i], mCold[i]);
            }
        }

        uint32_t GetLiveCount() const { return mLiveCount; }
        uint32_t GetCapacity() const { return mCapacity; }
        uint32_t GetFreeCount() const { return static_cast<uint32_t>(mFreeList.size()); }

    private:
        void Reserve(uint32_t capacity) {
            assert(capacity > mCapacity);

            mGenerations.resize(capacity, 0);
            mOccupied.resize(capacity, false);
            mHot.resize(capacity);
            mCold.resize(capacity);

            mCapacity = capacity;
        }

    private:
        std::vector<uint32_t> mFreeList;
        std::vector<uint32_t> mGenerations;
        std::vector<uint8_t> mOccupied;

        std::vector<THot> mHot;
        std::vector<TCold> mCold;


        uint32_t mCapacity = 0;
        uint32_t mNextFreshSlot = 1; // slot - is sentinel
        uint32_t mLiveCount = 0;
    };

}
