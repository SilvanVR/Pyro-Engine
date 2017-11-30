#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform UBO
{
	float exposure;
	float gamma;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec3 hdrColor = texture(Input0_S, inUV).rgb;
	
	// Reinhard tone mapping
	// vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
	  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

	outColor = vec4(mapped, 1.0);	
}

