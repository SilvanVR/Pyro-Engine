#version 450

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
} camera;

// Push-Constant for per object data
layout (std140, push_constant) uniform PushConstant 
{
	mat4 world;
} Object;

// In Data
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBiTangent;

// Out Data
layout (location = 0) out vec2 outUV;
layout (location = 1) out mat3 tbnMatrix;
layout (location = 4) out vec3 outWorldPos;


void main() 
{
	outUV 		= inUV;
	outWorldPos = (Object.world * vec4(inPos, 1.0)).xyz;
	gl_Position = camera.viewProjection * vec4(outWorldPos, 1.0);
	
	// Normal mapping calculation
	vec3 normal    = normalize((Object.world * vec4(inNormal, 0.0)).xyz);
	vec3 tangent   = normalize((Object.world * vec4(inTangent, 0.0)).xyz);
	vec3 biTangent = normalize((Object.world * vec4(inBiTangent, 0.0)).xyz);
	
	// Gramm Schmidt Process. It reorthogonalize the tangent, so the angle between the tangent and normal is perfectly 90°
	tangent = normalize(tangent - dot(tangent, normal) * normal);

	tbnMatrix = mat3(tangent, biTangent, normal);
}


















