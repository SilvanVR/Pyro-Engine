#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform UBO
{
	float brightnessThreshold;
};


// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec4 color = texture(Input0_S, inUV);
	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);

	if(brightness > brightnessThreshold)
		outColor = color;
	else
		outColor = vec4(0,0,0,1);
}

