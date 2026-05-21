cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project(VulkanRenderer)
# Set Build Type to Debug by default
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Set architecture to either 32 or 64 depending on system
set(ARCHITECTURE "32")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "64")
endif()

# OutputDir to be used for all projects
if (NOT CMAKE_CONFIGURATION_TYPES)
    set(OutputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
else()
    set(OutputDir "${CMAKE_SOURCE_DIR}/bin/")
endif()

file(GLOB_RECURSE SOURCE "${CMAKE_SOURCE_DIR}/src/**.cpp")
file(GLOB_RECURSE HEADER_SOURCE "${CMAKE_SOURCE_DIR}/header/**.h")
set(HEADER "${CMAKE_SOURCE_DIR}/header/")

file(GLOB_RECURSE GLAD_SRC "${CMAKE_SOURCE_DIR}/vendor/glad/**.c")
set(GLAD_HEADER "${CMAKE_SOURCE_DIR}/vendor/glad/include")

set(GLM_HEADER "${CMAKE_SOURCE_DIR}/vendor/glm/")
set(STB_HEADER "${CMAKE_SOURCE_DIR}/vendor/stb/")

find_package(Vulkan REQUIRED)

if (WIN32)
    find_library(SHADERC_LIB shaderc_shared HINTS $ENV{VULKAN_SDK}/Lib)
    set(VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/Include)
else()
    find_library(SHADERC_LIB shaderc_shared HINTS $ENV{VULKAN_SDK}/lib)
    set(VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/include)
endif()

message("Vulkan Path ${VULKAN_PATH}")

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/glfw/")
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/DearImGui/")

set(BIN_NAME "VKRE-${CMAKE_SYSTEM_NAME}-${ARCHITECTURE}")
add_executable(${BIN_NAME} "${SOURCE}" "${GLAD_SRC}" "${HEADER_SOURCE}")

target_link_libraries(${BIN_NAME} glfw)
target_link_libraries(${BIN_NAME} ImGui)
target_link_libraries(${BIN_NAME} ${SHADERC_LIB})
target_link_libraries(${BIN_NAME} Vulkan::Vulkan)

target_include_directories(${BIN_NAME} PUBLIC "${HEADER}" "${GLAD_HEADER}" "${GLM_HEADER}" "${STB_HEADER}" "${VULKAN_INCLUDE_PATH}")

if (WIN32)
    target_compile_definitions(${BIN_NAME} PRIVATE PLATFORM_WINDOWS=1)
elseif (APPLE)
    target_compile_definitions(${BIN_NAME} PRIVATE PLATFORM_MACOS=1)
elseif (UNIX)
    target_compile_definitions(${BIN_NAME} PRIVATE PLATFORM_LINUX=1)
endif()

set_target_properties(${BIN_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OutputDir}"
)

set_target_properties(${BIN_NAME} PROPERTIES 
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
