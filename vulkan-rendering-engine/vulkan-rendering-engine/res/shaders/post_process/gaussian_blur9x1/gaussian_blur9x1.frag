#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;

layout (set = 0, binding = 1) uniform UBO 
{
	float 	blurStrength;
	int 	horizontal;
	float 	blurScale;
} ubo;

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() 
{	
	vec2 tex_offset = 1.0 / textureSize(Input0_S, 0) * ubo.blurScale; // gets size of single texel
	vec3 result = texture(Input0_S, inUV).rgb * weight[0]; // current fragment's contribution
	for(int i = 1; i < 5; ++i)
	{
		if (ubo.horizontal == 1)
		{
			result += texture(Input0_S, inUV + vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * ubo.blurStrength;
			result += texture(Input0_S, inUV - vec2(tex_offset.x * i, 0.0)).rgb * weight[i] * ubo.blurStrength;
		}
		else
		{
			result += texture(Input0_S, inUV + vec2(0.0, tex_offset.y * i)).rgb * weight[i] * ubo.blurStrength;
			result += texture(Input0_S, inUV - vec2(0.0, tex_offset.y * i)).rgb * weight[i] * ubo.blurStrength;
		}
	}
	outColor = vec4(result, 1.0);
}

