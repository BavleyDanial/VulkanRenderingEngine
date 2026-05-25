#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "ResourceHandles.h"

namespace VKRE {

    enum class ShaderStage : uint8_t {
        None = 0,
        Vertex,
        Fragment,
        Compute,
    };

    struct ShaderDesc {
        std::vector<uint32_t> ByteCode;
        std::string DebugName = "";
        std::string Path = "";
        std::string Entrypoint = "main";
        ShaderStage Stage;
    };

    struct ShaderHotData {
        ShaderStage Stage;
    };

    struct ShaderColdData {
        char DebugName[64] = "";
        char Path[256] = "";
        char Entrypoint[32] = "main";
        std::vector<uint32_t> ByteCode;
        bool IsDirty = false;
    };

    enum class GPUBufferUsage : uint16_t {
        None = 0,
        Vertex      = 1 << 0,
        Index       = 1 << 1,
        Uniform     = 1 << 2,
        Storage     = 1 << 3,
        Indirect    = 1 << 4,
        TransferSrc = 1 << 5,
        TransferDst = 1 << 6,
    };

    struct GPUBufferDesc {
        std::string DebugName = "";
        uint64_t Size = 0;
        GPUBufferUsage Usage = GPUBufferUsage::None;
        bool HostVisible = false;
    };

    struct GPUBufferHotData {
        uint64_t DeviceAddress = 0;
        uint64_t Size = 0;
        bool HostVisible = false;
    };

    struct GPUBufferColdData {
        char DebugName[64] = "";
        GPUBufferUsage Usage = GPUBufferUsage::None;
    };

    struct Vertex {
        glm::vec3 Position;
        float UVx;
        glm::vec3 Normal;
        float UVy;
        glm::vec4 Color;
    };

    struct MeshDesc {
        std::string DebugName = "";
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    struct MeshHotData {
        GPUBufferHandle VertexBuffer = GPUBufferHandle::Null();
        GPUBufferHandle IndexBuffer = GPUBufferHandle::Null();
        uint32_t VerticesCount = 0;
        uint32_t IndicesCount = 0;
    };

    struct MeshColdData {
        char DebugName[64] = "";
    };

}

inline VKRE::GPUBufferUsage operator|(VKRE::GPUBufferUsage a, VKRE::GPUBufferUsage b) {
    return static_cast<VKRE::GPUBufferUsage>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

inline VKRE::GPUBufferUsage operator&(VKRE::GPUBufferUsage a, VKRE::GPUBufferUsage b) {
    return static_cast<VKRE::GPUBufferUsage>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}
