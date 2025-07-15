project "glm"
    kind "StaticLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "glm/*.hpp",
        "glm/simd/*",
        "glm/gtx/*",
        "glm/gtc/*",
        "glm/ext/*",
        "glm/detail/*",
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"