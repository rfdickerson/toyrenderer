cmake_minimum_required(VERSION 3.28)
project(toyrenderer)

add_subdirectory(vk-bootstrap)
add_subdirectory(VulkanMemoryAllocator)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)
#find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_executable(toyrenderer main.cpp
        vma_impl.cpp
        utils.hpp
        utils.cpp
        common.hpp
        stdafx.hpp)

target_link_libraries(toyrenderer
    PRIVATE
        glfw
        vk-bootstrap::vk-bootstrap
        vk-bootstrap-compiler-warnings
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
)

target_precompile_headers(
        toyrenderer
        PUBLIC
        stdafx.hpp
)
