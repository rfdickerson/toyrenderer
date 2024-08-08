//
// Created by rfdic on 7/24/2024.
//

#include "debug_utils.hpp"
#include "common.hpp"

namespace obsidian
{
void begin_debug_label(Init &init, const VkCommandBuffer &command_buffer, const char *label, glm::vec3 colors)
{
	VkDebugUtilsLabelEXT label_info = {};
	label_info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label_info.pLabelName           = label;
	label_info.color[0]             = colors.r;
	label_info.color[1]             = colors.g;
	label_info.color[2]             = colors.b;
	label_info.color[3]             = 1.0f;

	init.disp.cmdBeginDebugUtilsLabelEXT(command_buffer, &label_info);
}

void end_debug_label(Init &init, const VkCommandBuffer &command_buffer)
{
	init.disp.cmdEndDebugUtilsLabelEXT(command_buffer);
}

} // namespace obsidian