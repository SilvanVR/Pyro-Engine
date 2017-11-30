#version 450

out gl_PerVertex { 
     vec4 gl_Position;
};

// Defines
#define MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT 12 // HAS TO BE SAME AS IN phong.frag AND in code

// Uniforms
layout (set = 0, binding = 0) uniform ViewProjection
{
	mat4 viewProjection;
};

layout (set = 1, binding = 0) uniform ShadowMapViewProjection_S
{
	mat4 shadowMapViewProjection[MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT];
	int  NUM_SHADOW_MAPS_DIR_AND_SPOT;
};

layout (set = 1, binding = 2) uniform Camera_S
{
	vec3 position;
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
layout (location = 5) out vec4 outShadowCoords[MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT];

// Constants

// Transforms shadow-coords from (-1 to 1) to (0 to 1)
const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0,
	                      0.0, 0.5, 0.0, 0.0,
	                      0.0, 0.0, 1.0, 0.0,
	                      0.5, 0.5, 0.0, 1.0 );

//const float shadowDistance = 150.0f;
//const float transitionDistance = 10.0f;

const float density = 0.007;
const float gradient = 1.5;

void main() 
{
	outUV 		= inUV;
	outWorldPos = (Object.world * vec4(inPos, 1.0)).xyz;
	gl_Position = viewProjection * Object.world * vec4(inPos.xyz, 1.0);
	
	// Normal mapping calculation
	vec3 normal    = normalize((Object.world * vec4(inNormal, 0.0)).xyz);
	vec3 tangent   = normalize((Object.world * vec4(inTangent, 0.0)).xyz);
	vec3 biTangent = normalize((Object.world * vec4(inBiTangent, 0.0)).xyz);
	
	// Gramm Schmidt Process. It reorthogonalize the tangent, so the angle between the tangent and normal is perfectly 90°
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	
	//vec3 biTangent = cross(tangent, normal);	
	tbnMatrix = mat3(tangent, biTangent, normal);
	
	// Shadow Calculation
	for(int i = 0; i < NUM_SHADOW_MAPS_DIR_AND_SPOT; i++)
	{
		outShadowCoords[i] = ( biasMat * shadowMapViewProjection[i] * Object.world ) * vec4(inPos.xyz, 1.0);
	}
	
	float distanceToCam = length(camera.position - outWorldPos);
	

	//distanceToCam = distanceToCam - (shadowDistance - transitionDistance);
	//distanceToCam = distanceToCam / transitionDistance;
    //outShadowTransitionFactor = clamp(1.0 - distanceToCam, 0.0, 1.0);	
}


















