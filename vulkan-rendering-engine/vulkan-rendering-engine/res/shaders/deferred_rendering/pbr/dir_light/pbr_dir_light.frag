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

struct DirectionalLight
{
	BaseLight base;
	vec3 direction;
};

// In Data
layout (location = 0) in vec2 inUV;

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

layout (set = 2, binding = 0) uniform DIRECTIONALLIGHT
{
	DirectionalLight directionalLight;
	int 	renderShadows;
	float 	minVariance;
	float 	linStep;
	mat4 	shadowMapViewProjection;
};
layout (set = 2, binding = 1) uniform sampler2D shadowMap;


const float PI = 3.14159265359;

float getLightIntensity(){ return directionalLight.base.intensity; }
vec3  getLightColor(){ return directionalLight.base.color; }
vec3  getLightDirection(){ return directionalLight.direction; }

float linstep(float low, float high, float w)
{
	return clamp((w - low)/(high - low), 0.0, 1.0);
}

// Calculates Shadow using the Chebyshev's inequality (Variance Shadow Mapping)
float sampleVarianceShadowMap(sampler2D shadowMap, vec2 coords, float compare)
{
	vec2 moments = texture(shadowMap, coords).xy;
	
	float p = step(compare, moments.x);
	float variance = max(moments.y - moments.x * moments.x, minVariance);
	
	float d = compare - moments.x;
	float pMax = linstep(linStep, 1.0, variance / (variance + d*d));
	
	return min(max(p, pMax), 1.0);
}

bool inRange(float val)
{
	return val >= 0.01 && val < 0.99;
}

const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0,
	                      0.0, 0.5, 0.0, 0.0,
	                      0.0, 0.0, 1.0, 0.0,
	                      0.5, 0.5, 0.0, 1.0 );

// Calculates Shadow using the Chebyshev's inequality (Variance Shadow Mapping)
float ShadowCalculationVariance(vec3 fragPos)
{
	// Get fragment in light-space
	vec4 fragPosLightSpace = biasMat * shadowMapViewProjection * vec4(fragPos, 1.0);
	
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// If the shadowMap-coords in range, compare the calculated distance to the stored distance in the shadow-map
    if(inRange(projCoords.x) && inRange(projCoords.y) && inRange(projCoords.z))
        return sampleVarianceShadowMap(shadowMap, projCoords.xy, projCoords.z);
	else	
		return 1.0;
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
    vec3 N = normalize(normal);
    vec3 V = normalize(camera.position - fragPos);
	
	vec3 F0 = vec3(0.04); 
	F0      = mix(F0, albedo, metallic);
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);  
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
  
	kD *= 1.0 - metallic;	

	vec3 L = normalize(-getLightDirection());
    vec3 H = normalize(V + L);
  
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);       
	float G   = GeometrySmith(N, V, L, roughness); 
		
	vec3 nominator    = NDF * G * F;
	float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; 
	vec3 brdf         = nominator / denominator;  
      
	vec3 radiance = getLightColor() * getLightIntensity(); 
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
	// Get G-Buffer values
	float depth = texture(SamplerDepth, inUV).r;
	
	// Discard fragments close to zFar -> prevents shading of "nothing" or the skybox
	if(depth > 0.999999)
		discard;
	
	vec3 fragPos = worldPosFromDepth(depth, inUV);
		
	vec4 normalS = texture(SamplerNormal, inUV);
	vec4 diffuse = texture(SamplerAlbedo, inUV);
	
	float roughness = diffuse.a;
	float metallic  = normalS.a;
	
	vec3 albedo = diffuse.rgb;
	vec3 normal = normalS.rgb;
	
	// Shadow Calculation
	float shadow = 1.0;
	if(renderShadows > 0)
		shadow = ShadowCalculationVariance(fragPos);
		
	vec3 finalColor = calcLight(albedo, fragPos, normal, roughness, metallic);
	
	outColor = vec4(finalColor, 1.0) * shadow;
}

