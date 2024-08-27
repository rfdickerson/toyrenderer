#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	mat4 lightSpaceMatrix;
	vec3 lightDirection;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inTangent;
layout (location = 5) in vec3 inBitangent;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragNormal;
layout (location = 3) out vec4 fragPosLightSpace; 
layout (location = 4) out vec3 fragPos;
layout (location = 5) out mat3 TBN;

void main ()
{
	vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
	gl_Position = ubo.proj * ubo.view * worldPos;

	fragColor = inColor;
	fragTexCoord = inTexCoord;
	fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
	fragPosLightSpace = ubo.lightSpaceMatrix * worldPos;

	mat4 biasMatrix = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0
	);

	fragPosLightSpace = biasMatrix * fragPosLightSpace;

	fragPos = worldPos.xyz;

	// normal mapping
	vec3 T = normalize(vec3(ubo.model * vec4(inTangent, 0.0)));
	vec3 B = normalize(vec3(ubo.model * vec4(inBitangent, 0.0)));
	vec3 N = normalize(vec3(ubo.model * vec4(inNormal, 0.0)));
	
	TBN = mat3(T, B, N);


}