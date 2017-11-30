#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// This and the vertex-shader File contains important descriptor-sets
// which will be used across several shaders. The VulkanBase-Class creates
// a shader-object which loads these files and which creates appropriate 
// descriptor-set-layouts from the descriptor-sets within these files.

// Structs
struct BaseLight
{
	vec3 color;
	float intensity;
	vec3 position;
};

struct DirectionalLight
{
	BaseLight base;
	vec3 direction;
};

struct PointLight
{
	BaseLight base;
	vec3 attenuation;
};

struct SpotLight
{
	PointLight pointLight;
	vec3 direction;
	float cutoff;
};

// Out Data
layout(location = 0) out vec4 outColor;


// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
	mat4 viewMatInv;
	mat4 projMatInv;
} camera;

layout (set = 1, binding = 0) uniform sampler2D SamplerDepth;
layout (set = 1, binding = 1) uniform sampler2D SamplerNormal;
layout (set = 1, binding = 2) uniform sampler2D SamplerAlbedo;

layout (set = 2, binding = 0) uniform DIRECTIONALLIGHT
{
	DirectionalLight directionalLight;
	int 	renderShadows;
	float 	minVariance;
	float 	linStep;
	mat4 	shadowMapViewProjection;
} dl;
// Names have to be unique in this file. Character after "_" will be cut off during parsing.
// These are normally option-character ("S" for shader set etc.) so i take always "q", which has no meaning.
layout (set = 2, binding = 1) uniform sampler2D shadowMap_q;

layout (set = 3, binding = 0) uniform POINTLIGHT
{
	PointLight pointLight;
	int renderShadows;
} pl;
layout (set = 3, binding = 1) uniform samplerCube shadowMap_qq;

layout (set = 4, binding = 0) uniform SPOTLIGHT
{
	SpotLight spotLight;
	int 	renderShadows;
	float 	minVariance;
	float 	linStep;
	mat4 	shadowMapViewProjection;
};
layout (set = 4, binding = 1) uniform sampler2D shadowMap_qqq;



void main() 
{	
	outColor = vec4(0);
}

