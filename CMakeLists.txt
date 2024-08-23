cmake_minimum_required(VERSION 3.28)
project(toyrenderer)

#add_subdirectory(extern/vk-bootstrap)
#add_subdirectory(extern/VulkanMemoryAllocator)
#add_subdirectory(extern/KTX-Software)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Set the source directory
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Find glslc shader compiler
find_program(GLSLC glslc HINTS Vulkan::glslc)

# Function to compile shaders
function(compile_shader TARGET SHADER)
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
            COMMAND ${GLSLC} -o ${SPIRV} ${SHADER}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${SHADER_NAME}"
    )
    target_sources(${TARGET} PRIVATE ${SPIRV})
endfunction()

# Set the source directory
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui")

set(IMGUI_SRC
        ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.h
    ${IMGUI_DIR}/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui_impl_vulkan.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

)

add_executable(toyrenderer
        ${IMGUI_SRC}
        src/buffer.hpp
        src/camera.hpp
        src/camera.cpp
        src/common.hpp
        src/cube_map.hpp
        src/cube_map.cpp
        src/debug_utils.cpp
        src/debug_utils.hpp
        src/gui.hpp
        src/gui.cpp
        src/image_loader.cpp
        src/image_loader.hpp
        src/cube_map.cpp
        src/cube_map.hpp
        src/main.cpp
        src/mesh.cpp
        src/mesh.hpp
        src/obj_loader.cpp
        src/obj_loader.hpp
        src/shadow.cpp
        src/shadow.hpp
        src/utils.hpp
        src/utils.cpp
        src/vma_impl.cpp
        src/buffer.cpp
        src/image.cpp
        src/image.hpp
        src/render.cpp
        src/render.hpp
        src/vertex.hpp
        src/uniforms.hpp)

# Compile shaders
file(GLOB_RECURSE SHADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
)
foreach(SHADER ${SHADERS})
    compile_shader(toyrenderer ${SHADER})
endforeach()

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")

file(COPY ${ASSETS_SOURCE_DIR} DESTINATION ${CMAKE_BINARY_DIR})

# Copy compiled shaders to the build directory
add_custom_command(TARGET toyrenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:toyrenderer>/shaders"
)

target_include_directories(toyrenderer PRIVATE ${SRC_DIR})

target_link_libraries(toyrenderer
        PRIVATE
        glfw
        vk-bootstrap::vk-bootstrap
        vk-bootstrap::vk-bootstrap-compiler-warnings
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        KTX::ktx
        imgui::imgui
        assimp::assimp
        glm::glm
        spdlog::spdlog
)

#Update the path for the precompiled header
if(EXISTS "${SRC_DIR}/stdafx.hpp")
    target_precompile_headers(
            toyrenderer
            PUBLIC
            ${SRC_DIR}/stdafx.hpp
    )
endif()