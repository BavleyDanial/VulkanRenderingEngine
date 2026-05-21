file(GLOB_RECURSE VKRE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE VKRE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/header/*.h")

add_library("VKRE" STATIC ${VKRE_SOURCES} ${VKRE_HEADERS})

target_include_directories("VKRE" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/header"
    "${GLAD_HEADER}"
    "${GLM_HEADER}"
    "${STB_HEADER}"
    "${VULKAN_INCLUDE_PATH}"
)

target_link_libraries("VKRE" PUBLIC
    glfw
    ImGui
    ${SHADERC_LIB}
    Vulkan::Vulkan
)
