#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable


// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
} camera;

layout (set = 1, binding = 0) uniform UBO_M
{
	vec4 color;
};

// Out Data
layout(location = 0) out vec4 outColor;

void main() 
{	
	outColor = color;
}
















