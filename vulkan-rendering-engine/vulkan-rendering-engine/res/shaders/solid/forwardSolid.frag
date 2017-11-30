#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
};


// In Data
layout (location = 0) in vec3 inColor;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{
	outColor = vec4(inColor, 1.0); 
}