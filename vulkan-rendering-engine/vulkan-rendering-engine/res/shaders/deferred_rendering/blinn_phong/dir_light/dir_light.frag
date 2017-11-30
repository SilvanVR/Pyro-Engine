#version 450

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

// Functions
vec3 calcLight(BaseLight base, vec3 direction, vec3 normal, float specularIntensity, vec3 fragPos)
{
	float diffuseFactor = dot(normal, -direction);
	
	vec3 diffuseColor = vec3(0,0,0);
	vec3 specularColor = vec3(0,0,0);
	
	if (diffuseFactor > 0)
	{
		diffuseColor = base.color * base.intensity * diffuseFactor;
		
		vec3 directionToEye = normalize(camera.position - fragPos);
		//vec3 reflectDirection = normalize(reflect(direction, normal));
		vec3 halfDirection = normalize(directionToEye - direction);
		
		//float specularFactor = dot(directionToEye, reflectDirection);
		float specularFactor = dot(halfDirection, normal);
		specularFactor = pow(specularFactor, 32);
		
		if(specularFactor > 0)
		{ 
			specularColor = base.color * specularIntensity * specularFactor * base.intensity;
		}
	}
	
	return diffuseColor + specularColor;
}

vec3 calcDirectionalLight(DirectionalLight directionalLight, vec3 normal, float specular, vec3 fragPos)
{
	return calcLight(directionalLight.base, directionalLight.direction, normal, specular, fragPos);
}

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
	vec3 fragPos = worldPosFromDepth(depth, inUV);
	vec3 normal   = texture(SamplerNormal, inUV).rgb;
	vec4 albedo   = texture(SamplerAlbedo, inUV);
	float specular = albedo.a;
	
	// Lighting + Shadow Calculations
	// Somehow structs passed to this shader are different from the structs passed to the functions
	vec3 color 		= directionalLight.base.color;
	float intensity = directionalLight.base.intensity;
	vec3 position 	= directionalLight.base.position;
	vec3 dir 		= directionalLight.direction;		
		
	DirectionalLight dirLight = DirectionalLight(BaseLight(color, intensity, position), dir);
		
	// Shadow Calculation
	float shadow = 1.0;
	if(renderShadows > 0)
		shadow = ShadowCalculationVariance(fragPos);
	
	// Final color calculation
	outColor = vec4(albedo.xyz * (calcDirectionalLight(dirLight, normal, specular, fragPos) * shadow), 1.0);
}

