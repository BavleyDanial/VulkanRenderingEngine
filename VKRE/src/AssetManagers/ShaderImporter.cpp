#include <AssetsManagers/ShaderImporter.h>
#include <ResourceManager/ShaderCompiler.h>

#include <fstream>
#include <sstream>

#include <optional>

namespace VKRE {

    std::optional<ShaderImportResults> ShaderImporter::LoadFromFile(
        ResourceManager& manager,
        const std::filesystem::path& path,
        const ShaderImportingOptions& options) {

        std::optional<std::string> source = ReadFile(path);
        if (!source.has_value()) return std::nullopt;

        std::vector<ParsedStage> stages = ParseStages(source.value());
        if (stages.empty()) {
            std::println("ShaderImporter::LoadFromFile No #ShaderType directives found in '{}'", path.string());
            return std::nullopt;
        }

        ShaderImportResults results{};
        results.Name = path.filename().string();
        bool hasFailed = false;

        ShaderCompileOptions compileOptions{};
        compileOptions.Optimise = options.Optimize;
        compileOptions.GenerateDebugInfo = options.GenerateDebugInfo;
        compileOptions.EntryPoint = options.EntryPoint;

        for (const ParsedStage& parsed : stages) {
            std::string name = std::format("{}:{}", results.Name, static_cast<uint32_t>(parsed.stage));

            ShaderCompileResult compResult = ShaderCompiler::Compile(parsed.source, parsed.stage, name, compileOptions);
            if (!compResult.Succeeded()) {
                std::println("ShaderImporter::LoadFromFile Failed to compile stage, Skipping stage '{}' in '{}'", static_cast<uint32_t>(parsed.stage), results.Name);
                hasFailed = true;
                continue;
            }

            ShaderDesc desc{};
            desc.Stage = parsed.stage;
            desc.ByteCode = compResult.ByteCode;
            desc.DebugName = name;
            desc.Entrypoint = options.EntryPoint;
            desc.Path = path.string();

            ResourceRef<ShaderTag> shader = manager.CreateShader(desc);
            if (!shader.IsValid()) continue;

            switch (parsed.stage) {
                case ShaderStage::Vertex: results.VertexShader = shader; break;
                case ShaderStage::Fragment: results.FragmentShader = shader; break;
                case ShaderStage::Compute: results.ComputeShader = shader; break;
                default: break;
            }

        }

        if (!results.IsValid()) {
            std::println("ShaderImporter::LoadFromFile no valid stages found (path={})", path.string());
            return std::nullopt;
        }

        return results;
    }

    std::optional<std::string> ShaderImporter::ReadFile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::println("ShaderCompilation::LoadFromFile File not found '{}'", path.string());
            return std::nullopt;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            std::println("ShaderCompilation::LoadFromFile Could not open file '{}'", path.string());
            return std::nullopt;
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }


    std::vector<ShaderImporter::ParsedStage> ShaderImporter::ParseStages(const std::string& source) {
        std::vector<ParsedStage> result;
        std::string commonBlock;

        int currentStageIdx = -1;

        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            std::string_view trimmed(line);
            size_t start = trimmed.find_first_not_of(" \t");
            if (start != std::string::npos)
                trimmed.remove_prefix(start);

            if (trimmed.starts_with("#ShaderType")) {
                size_t spacePos = trimmed.find_first_of(" \t");
                if (spacePos == std::string::npos) {
                    std::println("ShaderCompiler::ParseStages #ShaderType missing stage type");
                    continue;
                }

                std::string_view token = trimmed.substr(spacePos);
                size_t tokenPos = token.find_first_not_of(" \t");
                if (tokenPos == std::string::npos) {
                    std::println("ShaderCompiler::ParseStages #ShaderType missing stage type");
                    continue;
                }

                token.remove_prefix(tokenPos);
                ShaderStage stage = ParseStageToken(token);
                if (stage == ShaderStage::None) {
                    std::println("ShaderCompiler::ParseStages #ShaderType stage type is not recognised {} -skipped", token);
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

    ShaderStage ShaderImporter::ParseStageToken(std::string_view token) {
        if (token == "Vertex")                  return ShaderStage::Vertex;
        if (token == "Fragment")                return ShaderStage::Fragment;
        if (token == "Compute")                 return ShaderStage::Compute;
        return ShaderStage::None;
    }

}
