#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Shader.h"

namespace MigTech
{
	// Windows DirectX version of the MigTech shader
	class DxShader : public Shader
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;

	public:
		ID3D11VertexShader* getVertexShader() { return m_vertexShader.Get(); }
		ID3D11PixelShader* getPixelShader() { return m_pixelShader.Get(); }
		ID3D11InputLayout* getInputLayout() { return m_inputLayout.Get(); }

	public:
		DxShader(ID3D11VertexShader* pvs, ID3D11InputLayout* pil, unsigned int hints);
		DxShader(ID3D11PixelShader* pps, unsigned int hints);
		virtual ~DxShader();

		virtual Type getType();
	};
}