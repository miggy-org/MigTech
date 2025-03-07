#pragma once

#include "../core/MigUtil.h"

namespace MigTech
{
	///////////////////////////////////////////////////////////////////////////
	// enums


	///////////////////////////////////////////////////////////////////////////
	// common structs

	class hres_error : public std::runtime_error
	{
	public:
		explicit hres_error(const std::string& message, HRESULT hres) : std::runtime_error(message)
		{ _hres = hres; }
		explicit hres_error(const char* message, HRESULT hres) : std::runtime_error(message)
		{ _hres = hres; }

		virtual const char* what() const
		{
			static std::string msg;
			msg = std::runtime_error::what();
			msg += " (";
			msg += MigUtil::intToString((int)_hres);
			msg += ")";
			return msg.c_str();
		}

	private:
		HRESULT _hres;
	};

	///////////////////////////////////////////////////////////////////////////
	// D3D versions of the common vertex data types

	// VDTYPE_POSITION
	struct D3DVertexPosition
	{
		DirectX::XMFLOAT3 pos;
	};

	// VDTYPE_POSITION_COLOR
	struct D3DVertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};

	// VDTYPE_POSITION_COLOR_TEXTURE
	struct D3DVertexPositionColorTexture
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 uv;
	};

	// VDTYPE_POSITION_NORMAL
	struct D3DVertexPositionNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
	};

	// VDTYPE_POSITION_NORMAL_TEXTURE
	struct D3DVertexPositionNormalTexture
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 uv;
	};

	// VDTYPE_POSITION_TEXTURE
	struct D3DVertexPositionTexture
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	// VDTYPE_POSITION_TEXTURE_TEXTURE
	struct D3DVertexPositionTextureTexture
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv1;
		DirectX::XMFLOAT2 uv2;
	};
}