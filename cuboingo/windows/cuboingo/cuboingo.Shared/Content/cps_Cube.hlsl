// A constant buffer that stores the basic column-major matrices for composing geometry.
cbuffer ShaderConstantBasic : register(b0)
{
	float4 color;
	float4 misc;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
	float3 lpos : POSITION;
	float3 norm : NORMAL;
};

// Texturing variables
sampler textureSampler0;
sampler textureSampler1;
Texture2D tex0;
Texture2D tex1;

float3 getReflectionRay(float3 eye, float3 pos, float3 norm)
{
	float3 dir;
	float f2ndoti;

	dir = normalize(pos - eye);
	f2ndoti = 2.0 * dot(norm, dir);
	return (dir - f2ndoti*norm);
}

float convertToUV(float arg)
{
	return (arg / 1.4142) + 0.5;
}

float2 getReflectionCoords(float3 eye, float3 pos, float3 norm)
{
	float3 ray;
	float2 uv;
	float ax, ay, az;
	
	ray = getReflectionRay(eye, pos, norm);
	ax = abs(ray.x);
	ay = abs(ray.y);
	az = abs(ray.z);
	if (ax > ay && ax > az)
	{
		uv.x = ray.y;
		uv.y = ray.z;
	}
	else if (ay > ax && ay > az)
	{
		uv.x = ray.x;
		uv.y = ray.z;
	}
	else
	{
		uv.x = ray.x;
		uv.y = ray.y;
	}

	uv.x = convertToUV(uv.x);
	uv.y = convertToUV(uv.y);

	return uv;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.uv.x, 0, input.uv.y, 1);
	float4 retCol = tex0.Sample(textureSampler0, input.uv)*input.col;
	if (misc.w > 0)
	{
		retCol *= (1 - misc.w);

		float2 uv = getReflectionCoords(float3(misc.x, misc.y, misc.z), input.lpos, input.norm);
		float4 reflCol = tex1.Sample(textureSampler1, uv)*input.col;
		retCol += misc.w*reflCol;
	}

	return retCol;
}
