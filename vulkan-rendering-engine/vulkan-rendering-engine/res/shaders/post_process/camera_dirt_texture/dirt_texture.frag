#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform sampler2D dirtTexture;
layout (set = 0, binding = 2) uniform UBO
{
   float visibility;
} ubo;

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec4 texColor = texture(Input0_S, inUV);
	vec4 dirtColor = texture(dirtTexture, inUV) * ubo.visibility;

	float alpha = dirtColor.a;
	//outColor = texColor * (1 - alpha) + dirtColor * alpha;	
	outColor = texColor + dirtColor;	
}

