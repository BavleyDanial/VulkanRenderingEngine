#pragma once

#include <ResourceManager/Resources.h>

#include <array>
#include <vector>
#include <optional>
#include <filesystem>

namespace VKRE {

    struct TextureImportOptions {
        bool GenerateMipMaps = false;
        bool FlipVertically = true;
        bool IsLinear = false;
    };

    struct Texture2DImportResults {
        std::string Name;
        std::vector<std::byte> Data;
        uint32_t Width;
        uint32_t Height;
        TextureFormat Format;
    };

    struct TextureCubeImportResults {
        std::string Name;
        std::array<std::vector<std::byte>, 6> Data; // +X, -X, +Y, -Y, +Z, -Z
        uint32_t Width;
        uint32_t Height;
        TextureFormat Format;
    };

    class TextureImporter {
    public:
        static std::optional<Texture2DImportResults> LoadTexture2DFromFile(
            const std::filesystem::path& path,
            const TextureImportOptions& options = {}
        );

        static std::optional<TextureCubeImportResults> LoadTextureCubeFromFiles(
            const std::array<std::filesystem::path, 6>& paths,
            const TextureImportOptions& options = {}
        );

        static std::optional<TextureCubeImportResults> LoadTextureCubeFromHorizontalCross(
            const std::filesystem::path& paths,
            const TextureImportOptions& options = {}
        );

    private:
        static void ExtractFace(const Texture2DImportResults& cross, uint32_t faceSize, uint32_t bytesPerPixel,
                            uint32_t cellX, uint32_t cellY, std::vector<std::byte>& outFace);
    };

}
