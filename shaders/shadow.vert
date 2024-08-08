#version 450

layout(location = 0) in vec3 inPosition;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMatrix;
    vec3 lightDirection;
} ubo;



void main() {
    gl_Position = ubo.lightSpaceMatrix * vec4(inPosition, 1.0);

}