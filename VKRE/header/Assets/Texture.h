#pragma once

#include <ResourceManager/ResourceRefs.h>

namespace VKRE {

    struct Texture2DAsset {
        std::string Name;
        std::string Path;

        uint32_t Width;
        uint32_t Height;
        uint32_t MipLevels;
        TextureFormat Format;

        ResourceRef<Texture2DTag> Texture;
    };

}
