// A constant buffer that stores the basic column-major matrices for composing geometry.
cbuffer ShaderConstantBasic : register(b0)
{
	float4 color;
	float4 misc;
	int4 config;
};

// A constant buffer that stores the basic column-major matrices for composing geometry.
cbuffer ShaderConstantMatrix : register(b1)
{
	matrix model;
	matrix view;
	matrix proj;
	matrix mvp;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	// transform the vertex position into projected space
	float4 pos = float4(input.pos, 1.0f);
	output.pos = mul(pos, mvp);

	// misc.x is the map index (-1 = no map, 0-7 otherwise)
	// config.x is the rendering pass (0 = final)
	if (misc.x != -1 && config.x == 0)
	{
		output.uv.x = 0.125*(misc.x + input.uv.x);
		output.uv.y = input.uv.y;
	}
	else
		output.uv.x = output.uv.y = -1;

	return output;
}
