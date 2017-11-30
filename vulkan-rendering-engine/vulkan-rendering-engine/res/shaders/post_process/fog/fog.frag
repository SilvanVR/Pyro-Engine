#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Descriptor-Sets
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform sampler2D Input1;
layout (set = 0, binding = 2) uniform UBO
{
	vec3 fogColor;
	float density;
	float gradient;
	float zNear;
	float zFar;
};

// depthSample from depthTexture.r, for instance
float linearDepth(float depthSample)
{
    depthSample   = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;

void main() 
{	
	float depth 	= linearDepth(texture(Input1, inUV).r);
	vec4 sceneColor = texture(Input0_S, inUV);
		
	// FOG-Calculation	
	float visibility = 1.0;
	
	// This excludes objects really far away from the camera (skybox, sun)
	if(depth < (zFar - 10) )
		visibility = clamp(exp(-pow(depth*density, gradient)), 0.0, 1.0);
	
	outColor = mix(vec4(fogColor,1.0), sceneColor, visibility);
}

