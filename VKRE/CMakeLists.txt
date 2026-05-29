file(GLOB_RECURSE VKRE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE VKRE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/header/*.h")

add_library("VKRE" STATIC ${VKRE_SOURCES} ${VKRE_HEADERS})

set(STB_HEADER "${CMAKE_SOURCE_DIR}/vendor/stb/")
set(GLM_HEADER "${CMAKE_SOURCE_DIR}/vendor/glm/")
set(GLAD_HEADER "${CMAKE_SOURCE_DIR}/vendor/glad/include")
set(FLECS_HEADER "${CMAKE_SOURCE_DIR}/vendor/flecs/include")
set(ASSIMP_HEADER "${CMAKE_SOURCE_DIR}/vendor/assimp/include")

target_include_directories("VKRE" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/header"
    "${GLAD_HEADER}"
    "${STB_HEADER}"
    "${GLM_HEADER}"
    "${FLECS_HEADER}"
    "${ASSIMP_HEADER}"
    "${VULKAN_INCLUDE_PATH}"
)

target_link_libraries("VKRE" PRIVATE
    assimp
    ${SHADERC_LIB}
)

target_link_libraries("VKRE" PUBLIC
    glfw
    ImGui
    flecs_static
    Vulkan::Vulkan
)

target_compile_definitions("VKRE" PUBLIC
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)
