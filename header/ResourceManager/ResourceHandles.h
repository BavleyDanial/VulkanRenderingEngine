#pragma once

#include <cstdint>
#include <functional>

namespace VKRE {

    template<typename Tag>
    struct ResourceHandle {
        static constexpr uint32_t INVALID_IDX = 0xFFFFF;
        static constexpr uint32_t INVALID_GEN = 0x000;

        uint32_t index      : 20;
        uint32_t generation : 12;

        bool IsValid() const { return index != INVALID_IDX; }
        bool operator==(const ResourceHandle&) const = default;
        bool operator!=(const ResourceHandle&) const = default;

        static ResourceHandle Null() { return { INVALID_IDX, INVALID_GEN }; }
    };

    // NOTE: Add any future resources here
    using ShaderHandle = ResourceHandle<struct ShaderTag>;
    using TextureHandle = ResourceHandle<struct TextureTag>;

}

namespace std {

    template<typename Tag>
    struct hash<VKRE::ResourceHandle<Tag>> {
        size_t operator()(const VKRE::ResourceHandle<Tag>& handle) const noexcept {
            uint32_t packed = (handle.generation << 20) | handle.index; // pack both generation and index and hash them together.
            return std::hash<uint32_t>{}(packed);
        }
    };

}
