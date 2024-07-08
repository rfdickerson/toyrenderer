#version 450
#extension GL_ARB_separate_shader_objects : enable

// Object uniform buffer
layout(set = 0, binding = 0) uniform ObjectUBO {
	mat4 model;
} objectUbo;

// Camera uniform buffer
layout(set = 1, binding = 0) uniform CameraUBO {
	mat4 view;
	mat4 proj;
} cameraUbo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = cameraUbo.proj * cameraUbo.view * objectUbo.model * vec4(inPosition, 1.0);
	fragColor = inColor;
}