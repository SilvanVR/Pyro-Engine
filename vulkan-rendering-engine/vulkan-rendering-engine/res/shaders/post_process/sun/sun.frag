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

layout (set = 1, binding = 0) uniform sampler2D diffuse_S;
layout (set = 1, binding = 1) uniform UBO
{
	vec3 sunColor;
	int toSRGB;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;

// Transform a SRGB-Color to linear-space
vec4 toLinear(vec4 inColor)
{
	return vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
}

void main() 
{	
	vec4 col = texture(diffuse_S, inUV);
	outColor = (toSRGB * toLinear(col) + (1-toSRGB) * col) * vec4(sunColor, 1);
}
















