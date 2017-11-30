#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

out gl_PerVertex {
	vec4 gl_Position; // will use gl_Position
};

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBiTangent;

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
} camera;

// Push-Constant for per object data
layout (std140, push_constant) uniform PushConstant 
{
	mat4 world;
} Object;

layout (location = 0) out vec3 outColor;

void main() 
{
	vec3 normal = normalize(Object.world * vec4(inNormal, 0.0)).xyz;
	vec3 directionToEye = normalize(camera.position - (Object.world * vec4(inPos, 1.0)).xyz);
	
	float colorFactor = dot(directionToEye, normal);

	outColor = vec3(1.0, 1.0, 1.0) * colorFactor;
	gl_Position = camera.viewProjection * Object.world * vec4(inPos.xyz, 1.0);
}