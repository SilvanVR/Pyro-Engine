#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable


// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
} camera;

layout (set = 1, binding = 0) uniform samplerCube SamplerCubeMap_M;

// In Data
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inWorldPos;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec3 I = normalize (inWorldPos - camera.position);
	vec3 R = reflect (I, normalize(inNormal));
	
	outColor = texture(SamplerCubeMap_M, R);
}
















