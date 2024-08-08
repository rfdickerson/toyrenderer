#version 450

layout(set = 0, binding = 2) uniform samplerCube cubemap;

layout(location = 0) in vec3 inTexCoords;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(cubemap, inTexCoords);
}