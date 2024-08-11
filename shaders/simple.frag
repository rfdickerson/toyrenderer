#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMatrix;
    vec3 lightDirection;
    float near_plane;
    float far_plane;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D cubeMap;
layout(binding = 3) uniform sampler2DShadow shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragPosLightSpace;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    float scale;
    bool useTexture;
} pushConstants;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    return 1.0 - texture(shadowMap, fragPosLightSpace.xyz);
}

void main() {

    vec3 color;
    if (pushConstants.useTexture) {
        // load the diffuse map texture
        color = texture(texSampler, fragTexCoord).rgb;
    } else {

        float pattern = mod(floor(fragTexCoord.x * pushConstants.scale) +
                            floor(fragTexCoord.y * pushConstants.scale), 2.0);
        if (pattern > 0.0) {
            color = vec3(0.2, 0.2, 0.2);
        } else {
            color = vec3(0.8, 0.8, 0.8);
        }

    }

    vec3 normal = normalize(fragNormal);
    vec3 lightColor = vec3(0.95, 0.95, 1.0);

    // Ambient light
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse light
    vec3 lightDir = normalize(-ubo.lightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Calculate shadow
    float shadow = ShadowCalculation(fragPosLightSpace);

    // Combine lighting components
    vec3 lighting = (ambient + (1.0 - shadow) * diffuse) * color;

    // Final color
    outColor = vec4(lighting, 1.0);


}