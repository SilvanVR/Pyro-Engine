#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Structs
struct BaseLight
{
	vec3 color;
	float intensity;
	vec3 position;	
};

struct PointLight
{
	BaseLight base;
	vec3 attenuation;	
};

// Out Data
layout(location = 0) out vec4 outColor;

// Descriptor-Sets
layout (set = 0, binding = 0) uniform CAMERA
{
	vec3 position;
	mat4 viewProjection;
	mat4 viewMatInv;
	mat4 projMatInv;
} camera;

layout (set = 1, binding = 0) uniform sampler2D SamplerDepth;
layout (set = 1, binding = 1) uniform sampler2D SamplerNormal;
layout (set = 1, binding = 2) uniform sampler2D SamplerAlbedo;

layout (set = 2, binding = 0) uniform POINTLIGHT
{
	PointLight pointLight;	
	int renderShadows;
};
layout (set = 2, binding = 1) uniform samplerCube shadowMap;

const float PI = 3.14159265359;

vec3  getLightPos(){ return pointLight.base.position; }
float getLightIntensity(){ return pointLight.base.intensity; }
vec3  getLightColor(){ return pointLight.base.color; }

float getAttenuation(float distance)
{
	float att = pointLight.attenuation.x +
		        pointLight.attenuation.y * distance +
		        pointLight.attenuation.z * distance * distance + 0.01f;
				
	return 1.0 / att;
}

// Calculates Shadow for a PointLight
float ShadowCalculationPointLight(vec3 lightPos, vec3 fragPos)
{
	vec3 ld = fragPos - lightPos;
	float currentDepth = length(ld);
	
	float sampledDist = texture(shadowMap, ld).r;
	
	// Check if fragment is in shadow
	float bias = 0.15;
	float shadow = (currentDepth - bias < sampledDist) ? 1.0 : 0.0;
	
	return shadow;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 calcLight(vec3 albedo, vec3 fragPos, vec3 normal, float roughness, float metallic)
{	
	vec3  lightPos   = getLightPos();
	float intensity  = getLightIntensity();
	vec3  lightColor = getLightColor();
	
    vec3 N = normalize(normal);
    vec3 V = normalize(camera.position - fragPos);
	
	vec3 F0 = vec3(0.04); 
	F0      = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);  
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
  
	kD *= 1.0 - metallic;	

	vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);
  
    float distance    = length(lightPos - fragPos);
    vec3 radiance     = lightColor * getAttenuation(distance) * intensity; 
		
	float NDF = DistributionGGX(N, H, roughness);       
	float G   = GeometrySmith(N, V, L, roughness); 
		
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; 
	vec3 brdf         = nominator / denominator;  
  
    float NdotL = max(dot(N, L), 0.0);        
    vec3 Lo = (kD * albedo / PI + brdf) * radiance * NdotL;

	return Lo;  
}

// Vulkan's z-Range is from 0 - 1
vec3 worldPosFromDepth(float depth, vec2 texCoords) 
{
    vec4 clipSpacePosition = vec4(texCoords * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpacePosition = camera.projMatInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = camera.viewMatInv * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main() 
{	
	// Get size from a g-buffer texture (doesnt matter which one)
	vec2 screenSize = textureSize(SamplerAlbedo, 0);
	
	// Calc UV-Coordinate
	vec2 uvCoords = gl_FragCoord.xy / screenSize;

	// Get G-Buffer values
	float depth = texture(SamplerDepth, uvCoords).r;
	vec3 fragPos = worldPosFromDepth(depth, uvCoords);
	vec4 normalS = texture(SamplerNormal, uvCoords);
	vec4 diffuse = texture(SamplerAlbedo, uvCoords);
	
	float roughness = diffuse.a;
	float metallic  = normalS.a;
	
	vec3 albedo = diffuse.rgb;
	vec3 normal = normalS.rgb;
	
	// Shadow Calculation
	float shadow = 1.0;
	if(renderShadows > 0)
		shadow = ShadowCalculationPointLight(pointLight.base.position, fragPos);
		
	vec3 finalColor = calcLight(albedo, fragPos, normal, roughness, metallic);
	
	outColor = vec4(finalColor, 1.0) * shadow;
}










