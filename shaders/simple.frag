#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

// Uniform buffer for lights
struct Light {
    vec4 position;  // w component can be used to differentiate between point (1.0) and directional (0.0) lights
    vec4 color;     // RGB color and intensity in alpha
    vec4 direction; // For spot and directional lights
    float radius;   // For attenuation calculations
    float innerCutoff; // For spot lights
    float outerCutoff; // For spot lights
    float padding;  // To ensure 16-byte alignment
};

layout(set = 2, binding = 0) uniform LightsUBO {
    Light lights[8];
    int numActiveLights;
    vec3 padding;
} lightsUbo;

void main () {
    outColor = lightsUbo.lights[0].color * vec4 (fragColor, 1.0);
}
