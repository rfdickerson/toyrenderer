cmake_minimum_required(VERSION 3.28)
project(toyrenderer)

add_subdirectory(vk-bootstrap)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(toyrenderer main.cpp)


target_link_libraries(toyrenderer
    PRIVATE
        glfw
        vk-bootstrap::vk-bootstrap
        vk-bootstrap-compiler-warnings
        Vulkan::Vulkan
)