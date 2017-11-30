#version 450

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
	mat4 viewMatInv;
	mat4 projMatInv;
} camera;

// Shader - Set
layout (set = 1, binding = 0) uniform SETTINGS_S
{
	vec3 ambientLight;
	int  renderNormalMaps;
	int  renderDispMaps;
	int  renderAlbedo;
	int  renderNormals;
	float alphaDiscardThreshold;
};

// Material Set
layout (set = 2, binding = 0) uniform sampler2D diffuse;
layout (set = 2, binding = 1) uniform Material_DM
{
	vec4  color;
	float specularIntensity;
	float uvScale;
	float dispScale;
	float dispBias;
} material;
layout (set = 2, binding = 2) uniform sampler2D normalMap;
layout (set = 2, binding = 3) uniform sampler2D dispMap;

// In Data
layout (location = 0) in vec2 inUV;
layout (location = 1) in mat3 inTbnMatrix;
layout (location = 4) in vec3 inWorldPos;

// Out Data
layout (location = 0) out vec4 outLightAcc;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outNormal;

// Transform a SRGB-Color to linear-space
vec4 toLinear(vec4 inColor)
{
	return vec4(pow(inColor.rgb, vec3(2.2)), inColor.a);
}

// MAIN
void main() 
{	
	// Calculate parallax tex-coords
	vec3 directionToEye = normalize(camera.position - inWorldPos);
	// z value has to be flipped.. again.. still dont know why
	directionToEye.z = -directionToEye.z;

	vec2 texCoords = inUV * material.uvScale + (directionToEye * inTbnMatrix).xy * 
                    (texture(dispMap, inUV * material.uvScale).r * material.dispScale + material.dispBias);
		
	vec3 normal = normalize(inTbnMatrix * (255.0/128.0 * texture(normalMap, texCoords).xyz - 1));
	outNormal   = vec4(normal, 1.0);
	
	outAlbedo   = toLinear(texture(diffuse, texCoords)) * material.color;
	
	// Discard all fragments below that threshold
	if(outAlbedo.a < alphaDiscardThreshold)
		discard;
		
	// Specular-Intensity is stored in the alpha-component for now (TODO: For Blending i have to store it somewhere else)
	outAlbedo.a = material.specularIntensity;
	
	outLightAcc = outAlbedo * vec4(ambientLight, 1.0);
	
	if(renderNormalMaps > 0)
		outLightAcc = texture(normalMap, inUV * material.uvScale);
		
	if(renderDispMaps > 0)
		outLightAcc = texture(dispMap, inUV * material.uvScale);
		
	if(renderAlbedo > 0)
		outLightAcc = outAlbedo;
		
	if(renderNormals > 0)
		outLightAcc = outNormal;
}	














