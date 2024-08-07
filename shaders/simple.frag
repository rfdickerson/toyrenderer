#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMatrix;
    vec3 lightDirection;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D cubeMap;
layout(binding = 3) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragPosLightSpace;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Check if projection is outside the shadow map
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check if the current frag pos is in shadow
    float bias = 0.005;

    // Check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;


    return shadow;
}

void main() {
    // Base color (you can use texture or a fixed color)
    // vec3 color = texture(texSampler, fragTexCoord).rgb;
    vec3 color = vec3(0.8, 0.01, 0.0);

    float scale = 10.0;
    float pattern = mod(floor(fragTexCoord.x * scale) + floor(fragTexCoord.y * scale), 2.0);
    if (pattern > 0.0) {
        color = vec3(0.2, 0.2, 0.2);
    } else {
        color = vec3(0.8, 0.8, 0.8);
    }

    vec3 normal = normalize(fragNormal);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

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
    //outColor = vec4(color, 1.0);

}