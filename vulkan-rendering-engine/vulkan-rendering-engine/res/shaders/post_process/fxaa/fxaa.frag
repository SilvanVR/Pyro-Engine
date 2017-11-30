#version 450

#extension GL_ARB_separate_shader_objects : enable 
#extension GL_ARB_shading_language_420pack : enable

// Uniforms
layout (set = 0, binding = 0) uniform sampler2D Input0_S;
layout (set = 0, binding = 1) uniform UBO
{
	float spanMax;
	float reduceMin;
	float reduceMul;
};

// In Data
layout (location = 0) in vec2 inUV;

// Out Data
layout(location = 0) out vec4 outColor;

void main() 
{	
	vec3 luma = vec3(0.299, 0.587, 0.114);
	
	vec2 texDim = textureSize(Input0_S, 0);
	float lumaTL = dot(luma, texture(Input0_S, inUV + vec2(-1.0, -1.0) / texDim).xyz);
	float lumaTR = dot(luma, texture(Input0_S, inUV + vec2(1.0, -1.0) / texDim).xyz);
	float lumaBL = dot(luma, texture(Input0_S, inUV + vec2(-1.0, 1.0) / texDim).xyz);
	float lumaBR = dot(luma, texture(Input0_S, inUV + vec2(1.0, 1.0) / texDim).xyz);
	float lumaM  = dot(luma, texture(Input0_S, inUV).xyz);
	
	// Determine direction from the Luminance
	vec2 dir;
	dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
	dir.y = (lumaTL + lumaBL) - (lumaTR + lumaBR);
	
	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (reduceMul * 0.25), reduceMin);
	float inverseDirAdjustment = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	
	dir = min(vec2(spanMax, spanMax), 
	          max(vec2(-spanMax, -spanMax), dir * inverseDirAdjustment)) / texDim;
			  
	vec3 result1 = 0.5 * (
		texture(Input0_S, inUV + (dir * vec2(1.0/3.0 - 0.5))).xyz +
		texture(Input0_S, inUV + (dir * vec2(2.0/3.0 - 0.5))).xyz);
	
	vec3 result2 = result1 * 0.5 + 0.25 * (
		texture(Input0_S, inUV + (dir * vec2(0.0/3.0 - 0.5))).xyz +
		texture(Input0_S, inUV + (dir * vec2(3.0/3.0 - 0.5))).xyz);
		
	float lumaMin = min(min(lumaM, min(lumaTL, lumaTR)), min(lumaBL, lumaBR));
	float lumaMax = max(max(lumaM, max(lumaTL, lumaTR)), max(lumaBL, lumaBR));
	float lumaResult2 = dot(luma, result2);
	
	if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		outColor = vec4(result1, 1.0);
	else
		outColor = vec4(result2, 1.0);
}

