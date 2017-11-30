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
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec4 outShadowCoords[MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT];

// Constants

// Transforms shadow-coords from (-1 to 1) to (0 to 1)
const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0,
	                      0.0, 0.5, 0.0, 0.0,
	                      0.0, 0.0, 1.0, 0.0,
	                      0.5, 0.5, 0.0, 1.0 );

//const float shadowDistance = 150.0f;
//const float transitionDistance = 10.0f;

void main() 
{
	outUV 		= inUV;
	outNormal 	= (Object.world * vec4(inNormal, 0.0)).xyz;
	outWorldPos = (Object.world * vec4(inPos, 1.0)).xyz;
	gl_Position = viewProjection * Object.world * vec4(inPos.xyz, 1.0);
	
	// Shadow Calculation
	for(int i = 0; i < NUM_SHADOW_MAPS_DIR_AND_SPOT; i++)
	{
		outShadowCoords[i] = ( biasMat * shadowMapViewProjection[i] * Object.world ) * vec4(inPos.xyz, 1.0);
	}

	//float distanceToCam = length(camera.position - outWorldPos);
	//distanceToCam = distanceToCam - (shadowDistance - transitionDistance);
	//distanceToCam = distanceToCam / transitionDistance;
    //outShadowTransitionFactor = clamp(1.0 - distanceToCam, 0.0, 1.0);	
}


















