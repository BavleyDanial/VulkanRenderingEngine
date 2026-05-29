#pragma once

#include <ResourceManager/ResourceRefs.h>

namespace VKRE {

    struct ShaderAsset {
        std::string Name;
        std::string Path;

        ResourceRef<ShaderTag> VertexShader;
        ResourceRef<ShaderTag> FragmentShader;
        ResourceRef<ShaderTag> ComputeShader;
    };

}
