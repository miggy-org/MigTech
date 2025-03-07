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
	float2 uv : TEXCOORD0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float2 getUVSet(float frame, float rowCount, float colCount, float2 uvInc)
{
	int row = (frame / colCount);
	int col = (frame % colCount);

	float u1 = (1 / colCount) * col;
	float v1 = (1 / rowCount) * row;
	float u2 = u1 + (1 / colCount);
	float v2 = v1 + (1 / rowCount);

	float2 uvRet;
	uvRet.x = (uvInc.x == 0 ? u1 : u2);
	uvRet.y = (uvInc.y == 0 ? v1 : v2);
	return uvRet;
}

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	// transform the vertex position into projected space
	float4 pos = float4(input.pos, 1.0f);
	output.pos = mul(pos, mvp);

	// misc.x is the frame index
	// misc.y is the row count
	// misc.z is the column count
	if (misc.y > 1 || misc.z > 1)
		output.uv = getUVSet(misc.x, misc.y, misc.z, input.uv);
	else
		output.uv = input.uv;

	return output;
}
