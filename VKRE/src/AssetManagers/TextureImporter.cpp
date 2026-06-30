#include <AssetsManagers/TextureImporter.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <optional>
#include <print>

namespace VKRE {

    std::optional<Texture2DImportResults> TextureImporter::LoadTexture2DFromFile(
        const std::filesystem::path& path,
        const TextureImportOptions& options) {

        if (!std::filesystem::exists(path)) {
            std::println("TextureImporter::LoadFromFile couldn't find path to Texture (path={})", path.string());
            return std::nullopt;
        }

        std::string name = path.stem().string();
        stbi_set_flip_vertically_on_load(options.FlipVertically);

        int width, height, channels;
        Texture2DImportResults results{};
        if (!stbi_is_hdr(path.string().c_str())) {
            stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) {
                std::println("TextureImporter::LoadFromFile couldn't load Texture (path={}) (reasnon={})",
                                path.string(), stbi_failure_reason());
                return std::nullopt;
            }

            uint64_t dataSize = width * height * 4 * sizeof(stbi_uc);
            results.Data.resize(dataSize);
            memcpy(results.Data.data(), pixels, dataSize);

            results.Format = options.IsLinear ? TextureFormat::R8G8B8A8_UNORM : TextureFormat::R8G8B8A8_SRGB;
            stbi_image_free(pixels);
        } else {
            float* pixels = stbi_loadf(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels)
                std::println("{}", stbi_failure_reason());

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

    std::optional<TextureCubeImportResults> TextureImporter::LoadTextureCubeFromFiles(
        const std::array<std::filesystem::path, 6>& paths,
        const TextureImportOptions& options) {

        TextureCubeImportResults results{};
        std::string name = paths[0].stem().string();
        TextureImportOptions crossOptions = options;
        crossOptions.FlipVertically = false;

        uint32_t i = 0;
        for (const auto& path : paths) {
            std::optional<Texture2DImportResults> face = TextureImporter::LoadTexture2DFromFile(path, crossOptions);
            if (!face.has_value()) {
                std::println("TextureImporter::LoadTextureCubeFromFile failed to load face (path={})", path.string());
                return std::nullopt;
            }

            if (i == 0) {
                results.Width = face->Width;
                results.Height = face->Height;
                results.Format = face->Format;
            } else if (face->Width != results.Width || face->Height != results.Height || face->Format != results.Format) {
                std::println("TextureImporter::LoadTextureCubeFromFile face (path={}) doesn't have same dimensions/fomrat of previous faces", i);
                return std::nullopt;
            }

            results.Data[i] = std::move(face->Data);
            i++;
        }

        return results;
    }

    std::optional<TextureCubeImportResults> TextureImporter::LoadTextureCubeFromHorizontalCross(
        const std::filesystem::path& path,
        const TextureImportOptions& options) {
        TextureImportOptions crossOptions = options;
        crossOptions.FlipVertically = false;

        std::optional<Texture2DImportResults> cross = LoadTexture2DFromFile(path, crossOptions);
        if (!cross.has_value()) {
            std::println("TextureImporter::LoadCubeFromCross failed to load (path={})", path.string());
            return std::nullopt;
        }

        if (cross->Width % 4 != 0 || cross->Height % 3 != 0) {
            std::println("TextureImporter::LoadCubeFromCross dimensions ({}x{}) aren't divisible into a 4x3 cross, (path={})",
                            cross->Width, cross->Height, path.string());
            return std::nullopt;
        }

        uint32_t faceSize = cross->Width / 4;
        if (faceSize != cross->Height / 3) {
            std::println("TextureImporter::LoadCubeFromCross face cells aren't square ({} vs {}) (path={})",
                            faceSize, cross->Height / 3, path.string());
            return std::nullopt;
        }

        uint32_t bytesPerPixel = (cross->Format == TextureFormat::R32G32B32A32_SFLOAT) ? 16 : 4;

        TextureCubeImportResults results{};
        results.Name = path.stem().string();
        results.Width = faceSize;
        results.Height = faceSize;
        results.Format = cross->Format;

        struct FaceCell { uint32_t face; uint32_t cellX, cellY; };
        constexpr FaceCell layout[6] = {
            { 0, 2, 1 }, // +X
            { 1, 0, 1 }, // -X
            { 2, 1, 0 }, // +Y
            { 3, 1, 2 }, // -Y
            { 4, 1, 1 }, // +Z
            { 5, 3, 1 }, // -Z
        };

        for (const auto& cell : layout)
            ExtractFace(cross.value(), faceSize, bytesPerPixel, cell.cellX, cell.cellY, results.Data[cell.face]);

        return results;
    }

    void TextureImporter::ExtractFace(const Texture2DImportResults& cross, uint32_t faceSize, uint32_t bytesPerPixel,
            uint32_t cellX, uint32_t cellY, std::vector<std::byte>& outFace) {

        outFace.resize(static_cast<size_t>(faceSize) * faceSize * bytesPerPixel);

        uint32_t crossRowBytes = cross.Width * bytesPerPixel;
        uint32_t faceRowBytes  = faceSize * bytesPerPixel;

        uint32_t startX = cellX * faceSize;
        uint32_t startY = cellY * faceSize;

        for (uint32_t row = 0; row < faceSize; row++) {
            const std::byte* srcRow = cross.Data.data() + (static_cast<size_t>(startY + row) * crossRowBytes) + (static_cast<size_t>(startX) * bytesPerPixel);
            std::byte* dstRow = outFace.data() + static_cast<size_t>(row) * faceRowBytes;
            memcpy(dstRow, srcRow, faceRowBytes);
        }
    }

}
