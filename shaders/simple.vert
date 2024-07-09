#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;

layout(set = 0, binding = 0) uniform ObjectUBO {
	mat4 model;
} objectUbo;

layout(set = 1, binding = 0) uniform CameraUBO {
	mat4 view;
	mat4 proj;
} cameraUbo;

void main() {
	gl_Position = cameraUbo.proj * cameraUbo.view * objectUbo.model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragNormal = mat3(transpose(inverse(objectUbo.model))) * inNormal;
	fragPos = vec3(objectUbo.model * vec4(inPosition, 1.0));
}