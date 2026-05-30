#include <AssetsManagers/TextureImporter.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <print>

namespace VKRE {

    std::optional<TextureImportResults> TextureImporter::LoadFromFile(
        const std::filesystem::path& path,
        const TextureImportOptions& options) {

        if (!std::filesystem::exists(path)) {
            std::println("TextureImporter::LoadFromFile couldn't find path to Texture (path={})", path.string());
            return std::nullopt;
        }

        std::string name = path.stem().string();
        stbi_set_flip_vertically_on_load(options.FlipVertically);

        int width, height, channels;
        TextureImportResults results{};
        if (!stbi_is_hdr(path.string().c_str())) {
            stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

            uint64_t dataSize = width * height * 4 * sizeof(stbi_uc);
            results.Data.resize(dataSize);
            memcpy(results.Data.data(), pixels, dataSize);

            results.Format = options.IsLinear ? TextureFormat::R8G8B8A8_UNORM : TextureFormat::R8G8B8A8_SRGB;
            stbi_image_free(pixels);
        } else {
            float* pixels = stbi_loadf(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

            uint64_t dataSize = width * height * 4 * sizeof(float);
            results.Data.resize(dataSize);
            memcpy(results.Data.data(), pixels, dataSize);

            results.Format = TextureFormat::R32G32B32A32_SFLOAT;
            stbi_image_free(pixels);
        }

        if (results.Data.empty()) {
            std::println("TextureImporter::LoadFromFile couldn't load Texture (path={})", path.string());
            return std::nullopt;
        }

        results.Name = name;
        results.Width = static_cast<uint32_t>(width);
        results.Height = static_cast<uint32_t>(height);

        return results;
    }

}
