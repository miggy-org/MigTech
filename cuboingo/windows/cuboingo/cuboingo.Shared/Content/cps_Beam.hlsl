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
	float2 uv : TEXCOORD0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 retCol = color;
	retCol.a = 0.5*input.uv.x*misc.x;

	if (misc.y > 0)
	{
		if (input.uv.y <= misc.x)
		{
			float range = 0.1;
			float base = misc.x - range;
			float val = (input.uv.y - base) / range;
			if (val > 0)
			{
				retCol.r = retCol.r + (1 - retCol.r)*val / 2;
				retCol.g = retCol.g + (1 - retCol.g)*val / 2;
				retCol.b = retCol.b + (1 - retCol.b)*val / 2;
				retCol.a *= (1 + val);
			}
		}
	}
	return retCol;
}
