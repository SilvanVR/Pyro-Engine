#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable


layout (set = 0, binding = 0) uniform sampler2D Texture_S;


// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout (location = 0) out vec4 outLightAcc;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outNormal;

// Transform a SRGB-Color to linear-space
vec4 toLinear(vec4 inColor)
{
	return vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
}

void main() 
{
	outLightAcc = vec4(toLinear(texture(Texture_S, inUV)).xyz, 1.0); 
}