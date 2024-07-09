#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec3 fragPos;

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
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightsUbo.lights[0].position.xyz - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightsUbo.lights[0].color.rgb * diff;

    vec3 ambient = 0.1 * fragColor;

    vec3 result = (ambient + diffuse) * fragColor;
    outColor = vec4(result, 1.0);
    //outColor = vec4(norm, 1.0);
}
