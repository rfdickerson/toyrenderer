cmake_minimum_required(VERSION 3.28)
project(toyrenderer)

add_subdirectory(extern/vk-bootstrap)
add_subdirectory(extern/VulkanMemoryAllocator)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

# Set the source directory
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Collect all the source files
file(GLOB_RECURSE SRC_FILES
        "${SRC_DIR}/*.cpp"
        "${SRC_DIR}/*.hpp"
)

add_executable(toyrenderer ${SRC_FILES})

target_include_directories(toyrenderer PRIVATE ${SRC_DIR})

target_link_libraries(toyrenderer
        PRIVATE
        glfw
        vk-bootstrap::vk-bootstrap
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)

# Update the path for the precompiled header
if(EXISTS "${SRC_DIR}/stdafx.hpp")
    target_precompile_headers(
            toyrenderer
            PUBLIC
            ${SRC_DIR}/stdafx.hpp
    )
endif()