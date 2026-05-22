#pragma once

#include "ResourceHandles.h"

#include <vector>
#include <cassert>
#include <concepts>
#include <print>

namespace VKRE {

    template<typename Tag, typename THot, typename TCold>
    class ResourcePool {
    private:
        struct SlotMetaData {
            uint32_t generation     : 12 = 1;
            uint32_t refCount       : 20 = 0;
            uint32_t densePosition  : 32 = 0;
        };

    public:
        void Init(uint32_t initCapacity = 64) {
            assert(mCapacity == 0 && "ResourcePool::Init called more than once");
            assert(initCapacity < ResourceHandle<Tag>::INVALID_IDX && "ResourcePool::Init called more than once");
            Reserve(initCapacity);
            mSlotMetaData[0].generation = ResourceHandle<Tag>::INVALID_GEN;
            mSlotMetaData[0].refCount = 0;
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
            SlotMetaData& slotMetaData = mSlotMetaData[index];
            assert(slotMetaData.refCount == 0 && "Allocated a slot that is already occupied");

            slotMetaData.refCount = 1;
            mLiveCount++;

            mActiveIndices.push_back(index);
            slotMetaData.densePosition= static_cast<uint32_t>(mActiveIndices.size() - 1);

            return ResourceHandle<Tag>{ index, slotMetaData.generation };
        }

        void AddRef(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) return;
            mSlotMetaData[handle.index].refCount++;
        }

        bool RemoveRef(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) return false;
            SlotMetaData& slotMetaData = mSlotMetaData[handle.index];
            return (--slotMetaData.refCount) == 0;
        }

        void Free(ResourceHandle<Tag> handle) {
            if (!IsValid(handle)) {
                std::println("ResourcePool::Release attempted to release a stale or invalid handle (index={}, gen={})", static_cast<uint32_t>(handle.index), static_cast<uint32_t>(handle.generation));
                return;
            }

            SlotMetaData& slotMetaData = mSlotMetaData[handle.index];
            uint32_t targetDenseIdx = static_cast<uint32_t>(slotMetaData.densePosition);
            uint32_t lastSparseIdx = mActiveIndices.back();

            mActiveIndices[targetDenseIdx] = lastSparseIdx;
            mSlotMetaData[lastSparseIdx].densePosition = targetDenseIdx;
            mActiveIndices.pop_back();

            slotMetaData.refCount = 0;
            mLiveCount--;

            slotMetaData.generation = (slotMetaData.generation + 1) & 0xFFF;
            if (slotMetaData.generation == ResourceHandle<Tag>::INVALID_GEN)
                slotMetaData.generation = 1;

            mFreeList.push_back(handle.index);
        }

        bool IsValid(ResourceHandle<Tag> handle) const {
            if (!handle.IsValid() || handle.index >= mCapacity) return false;
            return mSlotMetaData[handle.index].generation == handle.generation;
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
        requires std::invocable<Fn, const THot&, const TCold&>
        ResourceHandle<Tag> FindIf(Fn&& fn) const {
            for (uint32_t i : mActiveIndices) {
                if (fn(mHot[i], mCold[i])) {
                    return ResourceHandle<Tag>{ i, mSlotMetaData[i].generation };
                }
            }
            return ResourceHandle<Tag>::Null();
        }

        template<typename Fn>
        requires std::invocable<Fn, THot&, TCold&>
        ResourceHandle<Tag> FindIf(Fn&& fn) {
            for (uint32_t i : mActiveIndices) {
                if (fn(mHot[i], mCold[i])) {
                    return ResourceHandle<Tag>{ i, mSlotMetaData[i].generation };
                }
            }
            return ResourceHandle<Tag>::Null();
        }

        template<typename Fn>
        requires std::invocable<Fn, uint32_t, THot&, TCold&>
        void ForEach(Fn&& fn) {
            for (uint32_t i : mActiveIndices) {
                fn(i, mHot[i], mCold[i]);
            }
        }

        template<typename Fn>
        requires std::invocable<Fn, uint32_t, const THot&, const TCold&>
        void ForEach(Fn&& fn) const {
            for (uint32_t i : mActiveIndices) {
                fn(i, mHot[i], mCold[i]);
            }
        }

        uint32_t GetLiveCount() const { return mLiveCount; }
        uint32_t GetCapacity() const { return mCapacity; }
        uint32_t GetFreeCount() const { return static_cast<uint32_t>(mFreeList.size()); }

    private:
        void Reserve(uint32_t capacity) {
            assert(capacity > mCapacity);
            assert(capacity < ResourceHandle<Tag>::INVALID_IDX && "Pool capacity is over 20 bits in size!");

            mSlotMetaData.resize(capacity);
            mHot.resize(capacity);
            mCold.resize(capacity);

            mCapacity = capacity;
        }

    private:
        std::vector<SlotMetaData> mSlotMetaData;
        std::vector<uint32_t> mFreeList;
        std::vector<uint32_t> mActiveIndices;

        std::vector<THot> mHot;
        std::vector<TCold> mCold;


        uint32_t mCapacity = 0;
        uint32_t mNextFreshSlot = 1; // slot - is sentinel
        uint32_t mLiveCount = 0;
    };

}
