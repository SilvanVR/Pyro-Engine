#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform sampler2D tex;

layout (set = 0, binding = 1) uniform GUIElement_M
{
	vec4 color;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec4 texColor = texture(tex, inUV);	

	outColor = vec4(color.rgb, texColor.r * color.a);
}
















