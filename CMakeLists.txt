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

set(ARCHITECTURE ${ARCHITECTURE} CACHE INTERNAL "")
set(OutputDir ${OutputDir} CACHE INTERNAL "")

file(GLOB_RECURSE GLAD_SRC "${CMAKE_SOURCE_DIR}/vendor/glad/**.c")
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "Build assimp tests" FORCE)

find_package(Vulkan REQUIRED)

if (WIN32)
    find_library(SHADERC_LIB shaderc_shared HINTS $ENV{VULKAN_SDK}/Lib)
    set(VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/Include)
else()
    find_library(SHADERC_LIB shaderc_shared HINTS $ENV{VULKAN_SDK}/lib)
    set(VULKAN_INCLUDE_PATH $ENV{VULKAN_SDK}/include)
endif()

message("Vulkan: $ENV{VULKAN_SDK}")

if (WIN32)
    add_compile_definitions(PLATFORM_WINDOWS=1)
elseif (APPLE)
    add_compile_definitions(PLATFORM_MACOS=1)
elseif (UNIX)
    add_compile_definitions(PLATFORM_LINUX=1)
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/glfw/")
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/DearImGui/")
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/assimp/" SYSTEM)

add_subdirectory("VKRE")
add_subdirectory("Sandbox")
