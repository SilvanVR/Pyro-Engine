#version 450

layout(early_fragment_tests) in;

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

// Functions
vec3 calcLight(BaseLight base, vec3 direction, vec3 normal, float specularIntensity, vec3 fragPos)
{
	float diffuseFactor = dot(normal, -direction);
	
	vec3 diffuseColor = vec3(0,0,0);
	vec3 specularColor = vec3(0,0,0);
	
	if (diffuseFactor > 0)
	{
		diffuseColor = base.color * base.intensity * diffuseFactor;
		
		//vec3 directionToEye = normalize(camera.position - fragPos);
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

vec3 calcPointLight(PointLight pointLight, vec3 normal, float specularIntensity, vec3 fragPos)
{
	vec3 lightDirection = fragPos - pointLight.base.position;
	float distanceToPoint = length(lightDirection);
		
	lightDirection = normalize(lightDirection);
	
	vec3 color = calcLight(pointLight.base, lightDirection, normal, specularIntensity, fragPos);
	
	float attenuation = pointLight.attenuation.x +
						pointLight.attenuation.y * distanceToPoint +
						pointLight.attenuation.z * distanceToPoint * distanceToPoint +
						0.0001;
						
	return color / attenuation;
}

// Calculates Shadow for a PointLight
float ShadowCalculationPointLight(vec3 lightPos, vec3 fragPos)
{
	vec3 ld = fragPos - lightPos;
	float dist = length(ld);
		
	// Sample the distance from cubemap with flipped z (i dont know why z has to be flipped)
    float sampledDist = texture(shadowMap, vec3(ld.x, ld.y, -ld.z)).r;
   
	// Check if fragment is in shadow
    float shadow = (dist <= sampledDist + 0.15) ? 1.0 : 0.0;
	
	return shadow;
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
	vec3 normal = texture(SamplerNormal, uvCoords).rgb;
	vec4 albedo = texture(SamplerAlbedo, uvCoords);
	float specularIntensity = albedo.a;
	
	// Somehow structs passed to this shader are different from the structs passed to the functions
	vec3 color 			= pointLight.base.color;
	float intensity 	= pointLight.base.intensity;
	vec3 position 		= pointLight.base.position;
	vec3 atten 			= pointLight.attenuation;
	
	PointLight pointLight = PointLight(BaseLight(color, intensity, position), atten);
	
	// Shadow Calculation
	float shadow = 1.0;
	if(renderShadows > 0)
		shadow = ShadowCalculationPointLight(position, fragPos);

	// Final color calculation
	outColor = vec4(albedo.xyz * calcPointLight(pointLight, normal, specularIntensity, fragPos), 1.0) * shadow;
}

