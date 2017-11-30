#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform UBO
{
	float contrast;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;

void main() 
{	
	outColor = texture(Input0_S, inUV);
	outColor.rgb = (outColor.rgb - 0.5) * (1.0 + contrast) + 0.5;
}

