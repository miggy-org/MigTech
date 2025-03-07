#include "pch.h"
#include "DxShader.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

//using namespace D2D1;
//using namespace DirectX;
using namespace Microsoft::WRL;
//using namespace Windows::Foundation;
//using namespace Windows::Graphics::Display;
//using namespace Windows::UI::Core;
//using namespace Windows::UI::Xaml::Controls;
//using namespace Platform;

using namespace MigTech;

DxShader::DxShader(ID3D11VertexShader* pvs, ID3D11InputLayout* pil, unsigned int hints) :
	Shader(hints)
{
	m_vertexShader.Attach(pvs);
	m_inputLayout.Attach(pil);
}

DxShader::DxShader(ID3D11PixelShader* pps, unsigned int hints) :
	Shader(hints)
{
	m_pixelShader.Attach(pps);
}

DxShader::~DxShader()
{
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
}

Shader::Type DxShader::getType()
{
	if (m_vertexShader.Get() != nullptr)
		return SHADER_TYPE_VERTEX;
	else if (m_pixelShader.Get() != nullptr)
		return SHADER_TYPE_PIXEL;

	return SHADER_TYPE_NONE;
}
