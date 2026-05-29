#pragma once

#include <Assets/Shader.h>

#include <filesystem>

namespace VKRE {

    struct ShaderImportingOptions {
        bool Optimize = false;
        bool GenerateDebugInfo = true;
        std::string EntryPoint = "main";
    };

    struct ShaderImportResults {
        std::string Name;

        ResourceRef<ShaderTag> VertexShader;
        ResourceRef<ShaderTag> FragmentShader;
        ResourceRef<ShaderTag> ComputeShader;

        bool IsValid() const { return VertexShader.IsValid() || FragmentShader.IsValid() || ComputeShader.IsValid(); }
    };

    class ShaderImporter {
    public:
        static std::optional<ShaderImportResults> LoadFromFile(
            ResourceManager& manager,
            const std::filesystem::path& path,
            const ShaderImportingOptions& options = {}
        );

        // TODO: Do these
        /*
        static std::optional<ShaderImportResults> LoadFromBinary(
            ResourceManager& manager,
            const std::filesystem::path& path,
            const ShaderImportingOptions& options = {}
        );

        static std::optional<ShaderImportResults> LoadFromSource(
            ResourceManager& manager,
            const std::string_view& source,
            const ShaderImportingOptions& options = {}
        );*/

    private:
        struct ParsedStage {
            ShaderStage stage;
            std::string source;
        };

    private:
        static std::optional<std::string> ReadFile(const std::filesystem::path& path);
        static std::vector<ParsedStage> ParseStages(const std::string& source);
        static ShaderStage ParseStageToken(std::string_view token);

    };

}
