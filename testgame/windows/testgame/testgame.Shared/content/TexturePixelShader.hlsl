// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// Texturing variables
sampler textureSampler;
Texture2D tex0;

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.uv.x, 0, input.uv.y, 1);
	return tex0.Sample(textureSampler, input.uv);
}
