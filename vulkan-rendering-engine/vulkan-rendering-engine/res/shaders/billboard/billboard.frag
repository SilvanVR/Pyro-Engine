#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
};

layout (set = 1, binding = 0) uniform sampler2D BillboardTexture_M;

// Out Data
layout(location = 0) out vec4 outColor;

// In Data
layout (location = 0) in vec2 inUV;

vec4 toLinear(vec4 inColor)
{
	return vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
}

void main() 
{
	outColor = toLinear(texture(BillboardTexture_M, inUV)); 
}