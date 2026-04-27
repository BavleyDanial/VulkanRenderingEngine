#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace VKRE {

    enum class ShaderStage : uint8_t {
        None = 0,
        Vertex,
        Fragment,
        Compute,
    };

    struct ShaderHotData {
        std::vector<uint32_t> byteCode;
        ShaderStage stage;
        char entrypoint[32] = "main";
    };

    struct ShaderColdData {
        char debugName[64] = "";
        char path[256] = "";
        bool isDirty = false;
    };

    struct ShaderDesc {
        std::vector<uint32_t> byteCode;
        ShaderStage stage;
        std::string entrypoint = "main";
        std::string debugName = "";
        std::string path = "";
    };

}
