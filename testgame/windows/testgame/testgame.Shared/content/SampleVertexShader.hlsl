// A constant buffer that stores the basic column-major matrices for composing geometry.
cbuffer ShaderConstantBasic : register(b0)
{
	float4 color;
	float4 misc;
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
	float4 color : COLOR0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, mvp);
	output.pos = pos;

	// Pass the color through without modification.
	output.color = input.color * color;

	return output;
}
