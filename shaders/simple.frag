#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMatrix;
    vec3 lightDirection;
} ubo;
 
layout(binding = 1) uniform MaterialBufferObject {
    float roughness;
    float metallic;
} material;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D cubeMap;
layout(binding = 5) uniform sampler2DShadow shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragPosLightSpace;
layout(location = 4) in vec3 fragPos;
layout(location = 5) in mat3 TBN;

layout(location = 0) out vec4 outColor;

vec3 getNormal()
{
    vec3 tangentNormal = texture(normalMap, fragTexCoord).rgb;
    tangentNormal.xy = tangentNormal.rg * 2.0 - 1.0;
    tangentNormal.z = sqrt(1.0 - dot(tangentNormal.xy, tangentNormal.xy));
    return normalize(TBN * tangentNormal);
}

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

    color = texture(texSampler, fragTexCoord).rgb;

    vec3 normal = getNormal();
    //vec3 normal = normalize(fragNormal);

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

    outColor = vec4(normal * 0.5 + 0.5, 1.0);
    outColor = vec4(lighting, 1.0);


}