#pragma once

#include "Resources.h"

#include <filesystem>
#include <string>
#include <cstdint>

namespace VKRE {

    struct ShaderCompileOptions {
        bool Optimise = false;
        bool GenerateDebugInfo = true;

        std::string EntryPoint = "main";
        std::vector<std::string> Defines;
        std::vector<std::filesystem::path> IncludePaths;
    };

    struct ShaderCompileResult {
        std::vector<uint32_t> ByteCode;
        std::string ErrorMsg;

        bool Succeeded() const { return !ByteCode.empty(); }
        bool Failed() const { return ByteCode.empty(); }
    };

    class ShaderCompiler {
    public:
        static ShaderCompileResult Compile(const std::string& source,
            ShaderStage stage,
            const std::string& debugName,
            const ShaderCompileOptions& options = {}
        );
    };

}
