#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
	mat4 viewMatInv;
	mat4 projMatInv;
} camera;

layout (set = 1, binding = 0) uniform samplerCube Cubemap1_S;
layout (set = 1, binding = 1) uniform samplerCube Cubemap2;
layout (set = 1, binding = 2) uniform UBO
{
	vec3 fogColor;
	float blendFactor;
	float lowerLimit;
	float upperLimit;
	float mipLevel1;
	float mipLevel2;
	float fogDensity;
};

// In Data
layout (location = 0) in vec3 inUVW;

// Out Data
layout(location = 0) out vec4 outColor;

// Transform a SRGB-Color to linear-space
vec4 toLinear(vec4 inColor)
{
	return vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
}

void main() 
{	
	vec4 cubeMap1col = textureLod(Cubemap1_S, normalize(inUVW), mipLevel1);
	vec4 cubeMap2col = textureLod(Cubemap2, normalize(inUVW), mipLevel2);
	
	vec4 finalColor = mix(cubeMap1col, cubeMap2col, blendFactor);
	
	float fogFactor = (inUVW.y - lowerLimit) / (upperLimit - lowerLimit);
	fogFactor = 1 - clamp(fogFactor, 0.0, 1.0);	
		
	outColor = mix(toLinear(finalColor), vec4(fogColor, 1.0), fogFactor * fogDensity);
}
















