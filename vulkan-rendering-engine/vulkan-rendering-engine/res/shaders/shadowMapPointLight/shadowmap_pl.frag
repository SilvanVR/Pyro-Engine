#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// In Data
layout(location = 0) in vec2 inUV; // Will be used for discarding alpha fragments -> bind material aswell
layout (location = 1) in vec3 inWorldPos;

// Out Data
layout(location = 0) out vec2 outColor;

// Structs
struct BaseLight
{
	vec3 color;
	float intensity;
	vec3 position;	
};

struct PointLight
{
	BaseLight base;
	vec3 attenuation;
};

layout (set = 0, binding = 0) uniform POINTLIGHT
{
	PointLight pointLight;	
	int renderShadows;
};
layout (set = 0, binding = 1) uniform samplerCube shadowMap;

void main() 
{
	float depth = length(inWorldPos - pointLight.base.position);	
	//float depth = gl_FragCoord.z;
	
	float dx = dFdx(depth);
	float dy = dFdy(depth);
	float variance = depth * depth + 0.25 * (dx * dx + dy * dy);	
	
	outColor = vec2(depth, variance);
}