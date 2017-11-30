#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

//layout(early_fragment_tests) in;

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
	float ambientIntensity;
	float specularIntensity;
	int   renderNormalMaps;
	int   renderDispMaps;
	int   renderAlbedo;
	int   renderNormals;
	float alphaDiscardThreshold;	
	int   useIBL;
	float maxReflectionLOD;
};
layout (set = 1, binding = 1) uniform sampler2D   BRDFLut;
layout (set = 1, binding = 2) uniform samplerCube IrradianceMap;
layout (set = 1, binding = 3) uniform samplerCube EnvironmentMap;


// Material Set
layout (set = 2, binding = 0) uniform sampler2D diffuse;
layout (set = 2, binding = 1) uniform Material_M
{
	vec4  color;
	float uvScale;
	float dispScale;
	float dispBias;
	float metallic;
	float roughness;
	int useRoughnessMap;
	int useMetallicMap;
} material;
layout (set = 2, binding = 2) uniform sampler2D normalMap;
layout (set = 2, binding = 3) uniform sampler2D dispMap;
layout (set = 2, binding = 4) uniform sampler2D roughnessMap;
layout (set = 2, binding = 5) uniform sampler2D metallicMap;
layout (set = 2, binding = 6) uniform sampler2D aoMap;


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

float getRoughness(vec2 texCoords)
{
	return (1.0 - material.useRoughnessMap) * material.roughness + material.useRoughnessMap * texture(roughnessMap, texCoords).r;
}

float getMetallic(vec2 texCoords)
{
	return (1.0 - material.useMetallicMap) * material.metallic + material.useMetallicMap * texture(metallicMap, texCoords).r;
}


vec3 getNormal(vec2 texCoords)
{
	return (texture(normalMap, texCoords).rgb * 2 - 1).xyz;
}

float getAmbientOcclusion(vec2 texCoords)
{
	return texture(aoMap, texCoords).r;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 getIBL(vec3 fragPos, vec3 albedo, vec3 normal, float roughness, float metallic)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(camera.position - fragPos);
	vec3 R = reflect(-V, N); 
		
	vec3 F0 = vec3(0.04); 
	F0      = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);  
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;  
	kD *= 1.0 - metallic;	
	
	vec3 irradiance = toLinear(texture(IrradianceMap, N)).rgb;
	vec3 diffuse    = irradiance * albedo;	
		
    vec3 prefilteredColor = toLinear(textureLod(EnvironmentMap, R,  roughness * maxReflectionLOD)).rgb;    
    vec2 brdf = texture(BRDFLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
	vec3 ibl = (kD * diffuse) * ambientIntensity + specular * specularIntensity; 
	
	return ibl;
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
	
	vec3 normal = normalize(inTbnMatrix * getNormal(texCoords));
	outNormal   = vec4(normal, 1.0);
	
	outAlbedo   = toLinear(texture(diffuse, texCoords)) * material.color;
	
	// Discard all fragments below that threshold
	if(outAlbedo.a < alphaDiscardThreshold)
		discard;
		
	// Store roughness in the alpha-component of the albedo
	outAlbedo.a = getRoughness(texCoords);
	
	// Store metallic in the alpha-component of the normal
	outNormal.a = getMetallic(texCoords);
	
	
	// Prefill the light-acc target with the ambient result
	if(useIBL > 0)
		outLightAcc = vec4(getIBL(inWorldPos, outAlbedo.xyz, outNormal.xyz,
							      outAlbedo.a, outNormal.a) * getAmbientOcclusion(texCoords), 1.0);
    else
		outLightAcc = outAlbedo * vec4(vec3(ambientIntensity), 1.0);

	if(renderNormalMaps > 0)
		outLightAcc = texture(normalMap, inUV * material.uvScale);
		
	if(renderDispMaps > 0)
		outLightAcc = texture(dispMap, inUV * material.uvScale);
		
	if(renderAlbedo > 0)
		outLightAcc = outAlbedo;
		
	if(renderNormals > 0)
		outLightAcc = outNormal;
}	














