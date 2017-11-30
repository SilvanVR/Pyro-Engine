#version 450

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
} camera;

layout (set = 1, binding = 0) uniform sampler2D SamplerPosition;
layout (set = 1, binding = 1) uniform sampler2D SamplerNormal;
layout (set = 1, binding = 2) uniform sampler2D SamplerAlbedo;

layout (set = 2, binding = 0) uniform UBO_S
{
	float density;
	float gradient;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


void main() 
{	
	vec3 fragPos = texture(SamplerPosition, inUV).rgb;
	vec4 albedo = texture(SamplerAlbedo, inUV);
		
	// FOG-Calculation
	float distanceToCam = length(camera.position - fragPos);
	float visibility =  clamp(exp(-pow(distanceToCam*density, gradient)), 0.0, 1.0);
	
	vec3 fogColor = vec3(0.5, 0.5, 0.5);
	//outColor = mix(vec4(fogColor,1.0), albedo, visibility);
	outColor =vec4(fogColor,1 - visibility);
}

