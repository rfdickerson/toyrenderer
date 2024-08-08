//
// Created by rfdic on 7/24/2024.
//

#ifndef TOYRENDERER_DEBUG_UTILS_HPP
#define TOYRENDERER_DEBUG_UTILS_HPP

namespace obsidian
{
struct Init;

void begin_debug_label(Init& init, const VkCommandBuffer& command_buffer, const char* label, glm::vec3 colors);
void end_debug_label(Init& init, const VkCommandBuffer& command_buffer);

}

#endif        // TOYRENDERER_DEBUG_UTILS_HPP
