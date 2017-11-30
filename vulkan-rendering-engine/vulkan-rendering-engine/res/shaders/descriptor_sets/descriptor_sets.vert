#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// This and the fragment-shader File contains important descriptor-sets
// which will be used across several shaders. The VulkanBase-Class creates
// a shader-object which loads these files and which creates appropriate 
// descriptor-set-layouts from the descriptor-sets within these files.

out gl_PerVertex { 
     vec4 gl_Position;
};

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
	mat4 viewMatInv;
	mat4 projMatInv;
};

void main() 
{
	gl_Position = vec4(0);
}