#pragma once

#include <ResourceManager/Resources.h>

#include <cstddef>
#include <filesystem>
#include <optional>

namespace VKRE {

    struct TextureImportOptions {
        bool GenerateMipMaps = false;
        bool FlipVertically = true;
        bool IsLinear = false;
    };

    struct TextureImportResults {
        std::string Name;
        std::vector<std::byte> Data;
        uint32_t Width;
        uint32_t Height;
        TextureFormat Format;
    };

    class TextureImporter {
    public:
        static std::optional<TextureImportResults> LoadFromFile(
            const std::filesystem::path& path,
            const TextureImportOptions& options = {}
        );
    };

}
