#pragma once

#include "Resources.h"
#include "ResourceHandles.h"
#include "ResourceManager.h"

#include <filesystem>
#include <string>
#include <cstdint>

namespace VKRE {

    struct ShaderCompileOptions {
        bool optimise = false;
        bool generateDebugInfo = true;

        std::string entrypoint = "main";
        std::vector<std::string> defines;
        std::vector<std::filesystem::path> includePaths;
    };

    struct ShaderLoadingResults {
        std::vector<ShaderStage> stages;
        std::vector<ShaderHandle> handles;

        bool Succeeded() const { return !stages.empty() && stages.back() != ShaderStage::None; }

        ShaderHandle GetHandle(ShaderStage stage) const {
            for (size_t i = 0; i < stages.size(); i++) {
                if (stages[i] == stage) return handles[i];
            }
            return ShaderHandle::Null();
        }
    };

    class ShaderCompiler {
    public:
        static ShaderLoadingResults LoadFromFile(
            ResourceManager& manager,
            const std::filesystem::path& path,
            const std::string& debugName = "",
            const ShaderCompileOptions& options = {}
        );

        static ShaderHandle LoadFromSource(
            ResourceManager& manager,
            const std::string& source,
            ShaderStage stage,
            const std::string& debugName = "",
            const ShaderCompileOptions& options = {}
        );

        static ShaderHandle LoadPreCompiledFromFile(
            ResourceManager& manager,
            const std::filesystem::path& path,
            ShaderStage stage,
            const std::string& debugName = "",
            const std::string& entrypoint = "main"
        );

    private:
        struct ShaderCompileResult {
            std::vector<uint32_t> byteCode;
            std::string errorMsg;

            bool Succeeded() const { return !byteCode.empty(); }
            bool Failed() const { return byteCode.empty(); }
        };

        struct ParsedStage {
            ShaderStage stage;
            std::string source;
        };

    private:
        static std::vector<ParsedStage> ParseStages(const std::string& source);
        static ShaderStage ParseStageToken(const std::string& token);

        static ShaderCompileResult CompileStage(
            const std::string& source,
            ShaderStage stage,
            const std::string& debugName = "",
            const ShaderCompileOptions& options = {}
        );

        static ShaderHandle StoreInManager(
            ResourceManager& manager,
            std::vector<uint32_t>&& byteData,
            ShaderStage stage,
            const std::string& debugName,
            const std::string& entrypoint,
            const std::filesystem::path& path = ""
        );
    };

}
