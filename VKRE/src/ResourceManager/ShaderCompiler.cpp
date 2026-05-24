#include <ResourceManager/ShaderCompiler.h>

#include <ResourceManager/ResourceRefs.h>
#include <ResourceManager/Resources.h>

#include <shaderc/shaderc.hpp>

#include <sstream>
#include <fstream>
#include <string>
#include <print>

namespace VKRE {

    static shaderc_shader_kind ToShadercKind(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex:           return shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment:         return shaderc_glsl_fragment_shader;
            case ShaderStage::Compute:          return shaderc_glsl_compute_shader;
            case ShaderStage::None:             return shaderc_glsl_infer_from_source;
        }
    }

    ShaderLoadingResults ShaderCompiler::LoadFromFile(ResourceManager& manager, const std::filesystem::path& path, const std::string& debugName, const ShaderCompileOptions& options) {
        ShaderLoadingResults results{};

        if (!std::filesystem::exists(path)) {
            std::println("ShaderCompilation::LoadFromFile File not found '{}'", path.string());
            return results;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            std::println("ShaderCompilation::LoadFromFile Could not open file '{}'", path.string());
            return results;
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();

        std::vector<ParsedStage> stages = ParseStages(source);
        if (stages.empty()) {
            std::println("ShaderCompilation::LoadFromFile No #ShaderType directives found in '{}'", path.string());
            return results;
        }

        std::string fileName = path.filename().string();
        bool hasFailed = false;
        for (const ParsedStage& parsed : stages) {
            std::string name = std::format("{}:{}", debugName, static_cast<uint32_t>(parsed.stage));

            ShaderCompileResult compResult = CompileStage(parsed.source, parsed.stage, name, options);
            if (!compResult.Succeeded()) {
                std::println("ShaderCompilation::LoadFromFile Failed to compile stage, Skipping stage '{}' in '{}'", static_cast<uint32_t>(parsed.stage), fileName);
                hasFailed = true;
                continue;
            }

            ResourceRef<ShaderTag> shader = StoreInManager(manager, std::move(compResult.byteCode), parsed.stage, name, options.entrypoint, path);
            if (shader) {
                results.stages.push_back(parsed.stage);
                results.shaders.push_back(shader);
            }
        }

        if (hasFailed) {
            results.stages.push_back(ShaderStage::None);
            results.shaders.push_back(ResourceRef<ShaderTag>());
        }

        return results;
    }

    ResourceRef<ShaderTag> ShaderCompiler::LoadFromSource(ResourceManager& manager, const std::string& source, ShaderStage stage, const std::string& debugName, const ShaderCompileOptions& options) {
        ShaderCompileResult compResult = CompileStage(source, stage, debugName, options);
        if (!compResult.Succeeded()) {
            std::println("ShaderCompilation::LoadFromSource Failed to compile stage '{}' in '{}'", static_cast<uint32_t>(stage), debugName);
            return ResourceRef<ShaderTag>();
        }

        return StoreInManager(manager, std::move(compResult.byteCode), stage, debugName, options.entrypoint);
    }

    // TODO: read entrypoint automatically from precompiled file
    ResourceRef<ShaderTag> ShaderCompiler::LoadPreCompiledFromFile(ResourceManager& manager, const std::filesystem::path& path, ShaderStage stage, const std::string& debugName, const std::string& entrypoint) {
        static constexpr uint32_t SPIRV_MAGIC = 0x07230203;

        if (!std::filesystem::exists(path)) {
            std::println("ShaderCompiler::LoadPreCompiledFromFile file not found: '{}'", path.string());
            return ResourceRef<ShaderTag>();
        }

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::println("ShaderCompiler::LoadPreCompiledFromFile Could not open file: '{}'", path.string());
            return ResourceRef<ShaderTag>();
        }

        size_t byteSize = static_cast<size_t>(file.tellg());
        if (byteSize == 0 || byteSize % sizeof(uint32_t) != 0) {
            std::println("ShaderCompiler::LoadPreCompiledFromFile Invalid SPIR-V file size ({}) in: '{}'", byteSize, path.string());
            return ResourceRef<ShaderTag>();
        }

        file.seekg(0);
        std::vector<uint32_t> spirv(byteSize / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), static_cast<std::streamsize>(byteSize));

        if (spirv[0] != SPIRV_MAGIC) {
            std::println("ShaderCompiler: Invalid SPIRV magic in: {}", path.string());
            return ResourceRef<ShaderTag>();
        }

        return StoreInManager(manager, std::move(spirv), stage, debugName, entrypoint, path);
    }

    std::vector<ShaderCompiler::ParsedStage> ShaderCompiler::ParseStages(const std::string& source) {
        std::vector<ParsedStage> result;
        std::string commonBlock;

        int currentStageIdx = -1;

        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            std::string trimmed = line;
            size_t start = trimmed.find_first_not_of(" \t");
            if (start != std::string::npos)
                trimmed = trimmed.substr(start);

            if (trimmed.starts_with("#ShaderType")) {
                size_t spacePos = trimmed.find(' ');
                if (spacePos == std::string::npos) {
                    std::println("ShaderCompiler::ParseStages #ShaderType missing stage type");
                    continue;
                }

                std::string token = line.substr(spacePos + 1);
                ShaderStage stage = ParseStageToken(token);
                if (stage == ShaderStage::None) {
                    std::println("ShaderCompiler::ParseStages #ShaderType stage type is recognised {} -skipped", token);
                    continue;
                }

                for (const auto& existing : result) {
                    if (existing.stage == stage)
                        std::println("ShaderCompiler::ParseStages #ShaderType stage type is duplicated {} -compiling last one", token);
                }

                ParsedStage newStage{};
                newStage.stage = stage;
                newStage.source = commonBlock;

                result.push_back(std::move(newStage));
                currentStageIdx = static_cast<int>(result.size()) - 1;

                continue;
            }

            if (currentStageIdx == -1)
                commonBlock += line + '\n';
            else
                result[currentStageIdx].source += line + '\n';
        }

        return result;
    }

    ShaderStage ShaderCompiler::ParseStageToken(const std::string& token) {
        if (token == "Vertex")                  return ShaderStage::Vertex;
        if (token == "Fragment")                return ShaderStage::Fragment;
        if (token == "Compute")                return ShaderStage::Compute;
        return ShaderStage::None;
    }

    ShaderCompiler::ShaderCompileResult ShaderCompiler::CompileStage(const std::string& source, ShaderStage stage, const std::string& debugName, const ShaderCompileOptions& options) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions shadercOptions;

        shadercOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3); // TODO: Make this auto sync with VulkanContext's version
        shadercOptions.SetTargetSpirv(shaderc_spirv_version_1_6);
        shadercOptions.SetSourceLanguage(shaderc_source_language_glsl); // TODO: make this customizable
        shadercOptions.SetOptimizationLevel(options.optimise ? shaderc_optimization_level_performance : shaderc_optimization_level_zero);

        if (options.generateDebugInfo)
            shadercOptions.SetGenerateDebugInfo();

        for (const auto& define : options.defines) {
            auto pos = define.find('=');
            if (pos != std::string::npos) {
                shadercOptions.AddMacroDefinition(define.substr(0, pos), define.substr(pos + 1));
            } else {
                shadercOptions.AddMacroDefinition(define);
            }
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, ToShadercKind(stage), debugName.c_str(), shadercOptions);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::println("ShaderCompiler::CompileStage Failed to compile stage '{}' in '{}'\n{}", static_cast<uint32_t>(stage), debugName, result.GetErrorMessage());
            return { {}, result.GetErrorMessage() };
        }

        if (result.GetNumWarnings() > 0)
            std::println("ShaderCompiler::CompileStage warnings in stage '{}' in '{}' compiled with {} warning(s)", static_cast<uint32_t>(stage), debugName, result.GetErrorMessage());

        return { std::vector<uint32_t>(result.cbegin(), result.cend()), {} };
    }

    ResourceRef<ShaderTag> ShaderCompiler::StoreInManager(ResourceManager& manager, std::vector<uint32_t>&& byteData, ShaderStage stage, const std::string& debugName, const std::string& entrypoint, const std::filesystem::path& path) {
        size_t len = 0;

        ShaderDesc desc{};
        desc.Stage = stage;
        desc.Path = path.string();
        desc.DebugName = debugName;
        desc.Entrypoint = entrypoint;
        desc.ByteCode = std::move(byteData);

        return manager.LoadShader(std::move(desc));
    }
}

