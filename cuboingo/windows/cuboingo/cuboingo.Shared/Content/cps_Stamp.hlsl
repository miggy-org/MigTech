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
};

// Texturing variables
sampler textureSampler0;
Texture2D tex0;

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.uv.x, 0, input.uv.y, 1);
	return tex0.Sample(textureSampler0, input.uv)*input.col;
}
