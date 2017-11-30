#version 450

// Has to be the same as in code
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 1024
#define MAX_SPOT_LIGHTS 32
#define MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT 12 // HAS TO BE SAME AS IN in code
#define MAX_NUM_SHADOW_MAPS_POINT 8 // HAS TO BE SAME AS IN code

// Structs
struct BaseLight
{
	vec3 color;
	float intensity;
	vec3 position;	
	int samplerIndex; // Sampler-Index for a given light. -1 if shadows are disabled for this light.
};

struct Attenuation
{
	float constant;
	float linear;
	float exponent;	
	float range;
};

struct DirectionalLight
{
	BaseLight base;
	vec3 direction;
};

struct PointLight
{
	BaseLight base;
	Attenuation atten;
};

struct SpotLight
{
	PointLight pointLight;
	vec3 direction;
	float cutoff;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;


// Descriptor-Sets
layout (set = 0, binding = 0) uniform sampler2D SamplerPosition;
layout (set = 0, binding = 1) uniform sampler2D SamplerNormal;
layout (set = 0, binding = 2) uniform sampler2D SamplerAlbedo;
layout (set = 0, binding = 3) uniform sampler2D SamplerSpecular;

layout (set = 1, binding = 0) uniform SETTINGS
{
	int 	renderShadows;
	float 	minVariance;
	float 	linStep;
	int		renderPosition;
	int		renderNormals;
	int 	renderAlbedo;
	int 	renderSpecular;
};

layout (set = 1, binding = 1) uniform Camera_S
{
	vec3 position;
} camera;

layout (set = 1, binding = 2) uniform LightCounts_S
{
	int NUM_DIRECTIONAL_LIGHTS;
	int NUM_POINT_LIGHTS;
	int NUM_SPOT_LIGHTS;
} lightCounts;

layout (set = 1, binding = 3) uniform Lighting_S
{
	vec3 ambientLight;
	DirectionalLight 	directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight 			pointLights[MAX_POINT_LIGHTS];
	SpotLight 			spotLights[MAX_SPOT_LIGHTS];
} lighting;

layout (set = 1, binding = 4) uniform ShadowMapViewProjection_S
{
	mat4 shadowMapViewProjection[MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT];
};

layout (set = 1, binding = 5) uniform sampler2D 	ShadowMap_S[MAX_NUM_SHADOW_MAPS_DIR_AND_SPOT];
layout (set = 1, binding = 6) uniform samplerCube 	ShadowMapPointLight_S[MAX_NUM_SHADOW_MAPS_POINT];


// Functions
vec3 calcLight(BaseLight base, vec3 direction, vec3 normal, vec2 specular, vec3 fragPos)
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
		specularFactor = pow(specularFactor, specular.y);
		
		if(specularFactor > 0)
		{ 
			specularColor = base.color * specular.x * specularFactor * base.intensity;
		}
	}
	
	return diffuseColor + specularColor;
}

vec3 calcDirectionalLight(DirectionalLight directionalLight, vec3 normal, vec2 specular, vec3 fragPos)
{
	return calcLight(directionalLight.base, directionalLight.direction, normal, specular, fragPos);
}

vec3 calcPointLight(PointLight pointLight, vec3 normal, vec2 specular, vec3 fragPos)
{
	vec3 lightDirection = fragPos - pointLight.base.position;
	float distanceToPoint = length(lightDirection);
	
	if(distanceToPoint > pointLight.atten.range)
		return vec3(0,0,0);
		
	lightDirection = normalize(lightDirection);
	
	vec3 color = calcLight(pointLight.base, lightDirection, normal, specular, fragPos);
	
	float attenuation = pointLight.atten.constant +
						pointLight.atten.linear * distanceToPoint +
						pointLight.atten.exponent * distanceToPoint * distanceToPoint +
						0.0001;
						
	return color / attenuation;
}

vec3 calcSpotLight(SpotLight spotLight, vec3 normal, vec2 specular, vec3 fragPos)
{
	vec3 lightDirection = normalize(fragPos - spotLight.pointLight.base.position);
	float spotFactor = dot(lightDirection, spotLight.direction);
	
	vec3 color = vec3(0,0,0);
	
	if(spotFactor > spotLight.cutoff)
	{
		color = calcPointLight(spotLight.pointLight, normal, specular, fragPos) *
				(1.0 - (1.0 - spotFactor) / (1.0 - spotLight.cutoff));
	}
	
	return color;
}


float linstep(float low, float high, float w)
{
	return clamp((w - low)/(high - low), 0.0, 1.0);
}

// Calculates Shadow using the Chebyshev's inequality (Variance Shadow Mapping)
float sampleVarianceShadowMap(sampler2D shadowMap, vec2 coords, float compare)
{
	vec2 moments = texture(shadowMap, coords.xy).xy;
	
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
float ShadowCalculationVariance(int samplerIndex, vec3 lightPos, vec3 fragPos)
{
	// Get fragment in light-space
	vec4 fragPosLightSpace = biasMat * shadowMapViewProjection[samplerIndex] * vec4(fragPos, 1.0);
	
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Calculate distance from fragment to the light
	float dist = length(fragPos - lightPos);
	
	// If the shadowMap-coords in range, compare the calculated distance to the stored distance in the shadow-map
    if(inRange(projCoords.x) && inRange(projCoords.y) && inRange(projCoords.z))
        return sampleVarianceShadowMap(ShadowMap_S[samplerIndex], projCoords.xy, dist);
	else	
		return 1.0;
}

// Calculates Shadow for a PointLight
float ShadowCalculationPointLight(int samplerIndex, vec3 lightPos, float lightRange, vec3 fragPos)
{
	vec3 ld = fragPos - lightPos;
	float dist = length(ld);
	
	// Return "Lit" if fragment is not in light-range
	if(dist > lightRange)
		return 1.0;
		
	// Sample the distance from cubemap with flipped z (i dont know why z has to be flipped)
    float sampledDist = texture(ShadowMapPointLight_S[samplerIndex], vec3(ld.x, ld.y, -ld.z)).r;
   
	// Check if fragment is in shadow
    float shadow = (dist <= sampledDist + 0.15) ? 1.0 : 0.0;
	
	return shadow;
}

void main() 
{	
	// Get G-Buffer values
	vec3 fragPos = texture(SamplerPosition, inUV).rgb;
	vec3 normal = texture(SamplerNormal, inUV).rgb;
	vec4 albedo = texture(SamplerAlbedo, inUV);
	vec2 specular = texture(SamplerSpecular, inUV).rg;
	
	// Lighting + Shadow Calculations
	vec3 totalLight = vec3(0,0,0);
	for(int i = 0; i < lightCounts.NUM_DIRECTIONAL_LIGHTS; i++)
	{
		// Somehow structs passed to this shader are different from the structs passed to the functions
		vec3 color 			= lighting.directionalLights[i].base.color;
		float intensity 	= lighting.directionalLights[i].base.intensity;
		vec3 position 		= lighting.directionalLights[i].base.position;
		int samplerIndex 	= lighting.directionalLights[i].base.samplerIndex;
		vec3 dir 			= lighting.directionalLights[i].direction;		
		
		DirectionalLight dirLight = DirectionalLight(BaseLight(color, intensity, position, samplerIndex), dir);
		
		// Shadow Calculation
		float shadow = 1.0;
		if(samplerIndex >= 0 && renderShadows > 0)
			shadow = ShadowCalculationVariance(samplerIndex, position, fragPos);
		
		totalLight += calcDirectionalLight(dirLight, normal, specular, fragPos) * shadow;
	}	
			
	for(int i = 0; i < lightCounts.NUM_POINT_LIGHTS; i++)
	{
		// Somehow structs passed to this shader are different from the structs passed to the functions
		vec3 color 			= lighting.pointLights[i].base.color;
		float intensity 	= lighting.pointLights[i].base.intensity;
		vec3 position 		= lighting.pointLights[i].base.position;
		int samplerIndex 	= lighting.pointLights[i].base.samplerIndex;
		float constant 		= lighting.pointLights[i].atten.constant;
		float linear 		= lighting.pointLights[i].atten.linear;
		float exponent 		= lighting.pointLights[i].atten.exponent;
		float range 		= lighting.pointLights[i].atten.range;
		
		PointLight pointLight = PointLight(BaseLight(color, intensity, position, samplerIndex), 
		                                   Attenuation(constant, linear, exponent, range));
		
		// Shadow Calculation
		float shadow = 1.0;
		if(samplerIndex >= 0 && renderShadows > 0)
			shadow = ShadowCalculationPointLight(samplerIndex, position, range, fragPos);
		
		totalLight += calcPointLight(pointLight, normal, specular, fragPos) * shadow;
	}	
	
	for(int i = 0; i < lightCounts.NUM_SPOT_LIGHTS; i++)
    {
		// Somehow structs passed to this shader are different from the structs passed to the functions
		vec3 color 			= lighting.spotLights[i].pointLight.base.color;
		float intensity 	= lighting.spotLights[i].pointLight.base.intensity;
		vec3 position 		= lighting.spotLights[i].pointLight.base.position;
		int samplerIndex 	= lighting.spotLights[i].pointLight.base.samplerIndex;
		float constant 		= lighting.spotLights[i].pointLight.atten.constant;
		float linear 		= lighting.spotLights[i].pointLight.atten.linear;
		float exponent 		= lighting.spotLights[i].pointLight.atten.exponent;
		float range 		= lighting.spotLights[i].pointLight.atten.range;	
		vec3 direction  	= lighting.spotLights[i].direction;
		float cutoff 		= lighting.spotLights[i].cutoff;
		
		SpotLight spotLight = SpotLight(PointLight(BaseLight(color, intensity, position, samplerIndex), 
		                                Attenuation(constant, linear, exponent, range)), direction, cutoff);
										
		// Shadow Calculation
		float shadow = 1.0;
		if(samplerIndex >= 0 && renderShadows > 0)
			shadow = ShadowCalculationVariance(samplerIndex, position, fragPos);
										
		totalLight += calcSpotLight(spotLight, normal, specular, fragPos) * shadow;
	}	
		
	totalLight = lighting.ambientLight + totalLight.xyz;
	
	// Final color calculation
	outColor = albedo * vec4(totalLight, 1.0);

	if(renderPosition > 0)
		outColor = vec4(fragPos, 1.0);
		
	if(renderNormals > 0)
		outColor = vec4(normal, 1.0);
		
	if(renderAlbedo > 0)
		outColor = albedo;
		
	if(renderSpecular > 0)
		outColor = vec4(specular, 0.0, 1.0);
}

