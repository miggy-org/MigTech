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

// A constant buffer that stores the basic column-major matrices for composing geometry.
cbuffer ShaderConstantLights : register(b2)
{
	float4 ambientCol;
	float4 litCol[4];
	float4 litDirPos[4];
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
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
	float3 lpos : POSITION;
	float3 norm : NORMAL;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	// transform the vertex position into projected space
	float4 pos = float4(input.pos, 1.0f);
	output.pos = mul(pos, mvp);

	// single light (assumed to be a directional light, white color)
	float4 norm = mul(float4(input.norm, 0.0f), model);
	float dotProd = -dot(float3(litDirPos[0].x, litDirPos[0].y, litDirPos[0].z), float3(norm.x, norm.y, norm.z));

	// adjust color and pass through the UV coords
	output.col = float4(color.r*dotProd, color.g*dotProd, color.b*dotProd, color.a);
	output.norm = float3(norm.x, norm.y, norm.z);
	output.lpos = float3(output.pos.x, output.pos.y, output.pos.z);
	output.uv = input.uv;

	return output;
}
