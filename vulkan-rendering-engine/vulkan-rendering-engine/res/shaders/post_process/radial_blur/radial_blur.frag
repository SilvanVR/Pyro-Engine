#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform UBO 
{
	float radialBlurScale;
	float radialBlurStrength;
	vec2  radialOrigin;
} ubo;

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	ivec2 texDim = textureSize(Input0_S, 0);
	vec2 radialSize = vec2(1.0 / texDim.s, 1.0 / texDim.t); 
	
	vec2 UV = inUV;
 
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	UV += radialSize * 0.5 - ubo.radialOrigin;
 
	#define samples 150

	for (int i = 0; i < samples; i++) 
	{
		float scale = 1.0 - ubo.radialBlurScale * (float(i) / float(samples-1));
		color += texture(Input0_S, UV * scale + ubo.radialOrigin);
	}
 
	outColor = (color / samples) * ubo.radialBlurStrength;
}

