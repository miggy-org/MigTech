#include "pch.h"
#include "DxDefines.h"
#include "DxObject.h"
#include "DxRender.h"
#include "../core/MigUtil.h"

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

DxObject::DxObject() :
	_vertexStride(0),
	_vertexOffset(0),
	_indexCount(0),
	_indexOffset(0),
	_indexOffsetCount(0),
	_topology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED),
	_mappings(MAX_TEXTURE_MAPS),
	_inRenderSet(false)
{
}

DxObject::~DxObject()
{
	_vertexBuffer.Reset();
	_indexBuffer.Reset();
}

int DxObject::addShaderSet(const std::string& vs, const std::string& ps)
{
	ShaderSet newSet;

	newSet.vertexShader = (DxShader*)MigUtil::theRend->getShader(vs);
	if (newSet.vertexShader == nullptr)
		throw std::invalid_argument("(DxObject::addShaderSet) Vertex shader doesn't exist");
	if (newSet.vertexShader->getType() != Shader::SHADER_TYPE_VERTEX)
		throw std::invalid_argument("(DxObject::addShaderSet) Specified shader isn't a vertex shader");

	newSet.pixelShader = (DxShader*)MigUtil::theRend->getShader(ps);
	if (newSet.pixelShader == nullptr)
		throw std::invalid_argument("(DxObject::addShaderSet) Pixel shader doesn't exist");
	if (newSet.pixelShader->getType() != Shader::SHADER_TYPE_PIXEL)
		throw std::invalid_argument("(DxObject::addShaderSet) Specified shader isn't a pixel shader");

	_shaderSets.push_back(newSet);
	return _shaderSets.size() - 1;
}

static D3D11_FILTER toDxFilter(TXT_FILTER minFilter, TXT_FILTER magFilter)
{
	if (magFilter == TXT_FILTER_NEAREST)
	{
		switch (minFilter)
		{
		case TXT_FILTER_LINEAR: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case TXT_FILTER_NEAREST_MIPMAP_NEAREST: return D3D11_FILTER_MIN_MAG_MIP_POINT;
		case TXT_FILTER_LINEAR_MIPMAP_NEAREST: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case TXT_FILTER_NEAREST_MIPMAP_LINEAR: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case TXT_FILTER_LINEAR_MIPMAP_LINEAR: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	else if (magFilter == TXT_FILTER_LINEAR)
	{
		switch (minFilter)
		{
		case TXT_FILTER_LINEAR: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TXT_FILTER_NEAREST_MIPMAP_NEAREST: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case TXT_FILTER_LINEAR_MIPMAP_NEAREST: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case TXT_FILTER_NEAREST_MIPMAP_LINEAR: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case TXT_FILTER_LINEAR_MIPMAP_LINEAR: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	}
	else
		throw std::invalid_argument("(DxObject::toDxFilter) Invalid mag filter");
}

static D3D11_TEXTURE_ADDRESS_MODE toDxWrap(TXT_WRAP wrap)
{
	switch (wrap)
	{
	case TXT_WRAP_CLAMP: return D3D11_TEXTURE_ADDRESS_CLAMP;
	case TXT_WRAP_MIRRORED_REPEAT: return D3D11_TEXTURE_ADDRESS_MIRROR;
	}
	return D3D11_TEXTURE_ADDRESS_WRAP;
}

void DxObject::setImage(int index, const std::string& name, TXT_FILTER minFilter, TXT_FILTER magFilter, TXT_WRAP wrap)
{
	if (index < 0 || index > MAX_TEXTURE_MAPS)
		throw std::invalid_argument("(DxObject::setImage) Invalid index");
	if (minFilter == TXT_FILTER_NONE || magFilter == TXT_FILTER_NONE)
		throw std::invalid_argument("(DxObject::setImage) Invalid filter");
	if (wrap == TXT_WRAP_NONE)
		throw std::invalid_argument("(DxObject::setImage) Invalid wrap");

	_mappings[index].pimg = (DxImage*)MigUtil::theRend->getImage(name);
	if (_mappings[index].pimg == nullptr)
		throw std::invalid_argument("(DxObject::setImage) Invalid image");

	// TODO: we only support 24 distinct varities, may want to use a map to reduce sampler states
	D3D11_SAMPLER_DESC desc;
	desc.Filter = toDxFilter(minFilter, magFilter);
	desc.AddressU = toDxWrap(wrap);
	desc.AddressV = toDxWrap(wrap);
	desc.AddressW = toDxWrap(wrap);
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;
	desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 1;

	DxRender* pdr = (DxRender*)MigUtil::theRend;
	ID3D11Device1* d3dDevice = pdr->GetD3DDevice();
	HRESULT hres = d3dDevice->CreateSamplerState(&desc, &_mappings[index].pstate);
	if (hres != S_OK)
		throw hres_error("(DxObject::setImage) Could not create sampler state", hres);
}

DirectX::XMFLOAT2 toD3DFloat2(const Vector2& src)
{
	return DirectX::XMFLOAT2(src.x, src.y);
}

DirectX::XMFLOAT3 toD3DFloat3(const Vector3& src)
{
	return DirectX::XMFLOAT3(src.x, src.y, src.z);
}

DirectX::XMFLOAT4 toD3DFloat4(const Color& src)
{
	return DirectX::XMFLOAT4(src.r, src.g, src.b, src.a);
}

void* toD3DVertexData(const void* pdata, VDTYPE vdType, unsigned int count, unsigned int stride)
{
	byte* pout = nullptr;
	switch (vdType)
	{
	case VDTYPE_POSITION:					pout = (byte*) new D3DVertexPosition[count]; break;
	case VDTYPE_POSITION_COLOR:				pout = (byte*) new D3DVertexPositionColor[count]; break;
	case VDTYPE_POSITION_COLOR_TEXTURE:		pout = (byte*) new D3DVertexPositionColorTexture[count]; break;
	case VDTYPE_POSITION_NORMAL:			pout = (byte*) new D3DVertexPositionNormal[count]; break;
	case VDTYPE_POSITION_NORMAL_TEXTURE:	pout = (byte*) new D3DVertexPositionNormalTexture[count]; break;
	case VDTYPE_POSITION_TEXTURE:			pout = (byte*) new D3DVertexPositionTexture[count]; break;
	case VDTYPE_POSITION_TEXTURE_TEXTURE:	pout = (byte*) new D3DVertexPositionTextureTexture[count]; break;
	}

	for (unsigned int i = 0; i < count; i++)
	{
		switch (vdType)
		{
		case VDTYPE_POSITION:
			((D3DVertexPosition*)pout)[i].pos = toD3DFloat3(((VertexPosition*)pdata)[i].pos);
			break;
		case VDTYPE_POSITION_COLOR:
			((D3DVertexPositionColor*)pout)[i].pos = toD3DFloat3(((VertexPositionColor*)pdata)[i].pos);
			((D3DVertexPositionColor*)pout)[i].color = toD3DFloat4(((VertexPositionColor*)pdata)[i].color);
			break;
		case VDTYPE_POSITION_COLOR_TEXTURE:
			((D3DVertexPositionColorTexture*)pout)[i].pos = toD3DFloat3(((VertexPositionColorTexture*)pdata)[i].pos);
			((D3DVertexPositionColorTexture*)pout)[i].color = toD3DFloat4(((VertexPositionColorTexture*)pdata)[i].color);
			((D3DVertexPositionColorTexture*)pout)[i].uv = toD3DFloat2(((VertexPositionColorTexture*)pdata)[i].uv);
			break;
		case VDTYPE_POSITION_NORMAL:
			((D3DVertexPositionNormal*)pout)[i].pos = toD3DFloat3(((VertexPositionNormal*)pdata)[i].pos);
			((D3DVertexPositionNormal*)pout)[i].norm = toD3DFloat3(((VertexPositionNormal*)pdata)[i].norm);
			break;
		case VDTYPE_POSITION_NORMAL_TEXTURE:
			((D3DVertexPositionNormalTexture*)pout)[i].pos = toD3DFloat3(((VertexPositionNormalTexture*)pdata)[i].pos);
			((D3DVertexPositionNormalTexture*)pout)[i].norm = toD3DFloat3(((VertexPositionNormalTexture*)pdata)[i].norm);
			((D3DVertexPositionNormalTexture*)pout)[i].uv = toD3DFloat2(((VertexPositionNormalTexture*)pdata)[i].uv);
			break;
		case VDTYPE_POSITION_TEXTURE:
			((D3DVertexPositionTexture*)pout)[i].pos = toD3DFloat3(((VertexPositionTexture*)pdata)[i].pos);
			((D3DVertexPositionTexture*)pout)[i].uv = toD3DFloat2(((VertexPositionTexture*)pdata)[i].uv);
			break;
		case VDTYPE_POSITION_TEXTURE_TEXTURE:
			((D3DVertexPositionTextureTexture*)pout)[i].pos = toD3DFloat3(((VertexPositionTextureTexture*)pdata)[i].pos);
			((D3DVertexPositionTextureTexture*)pout)[i].uv1 = toD3DFloat2(((VertexPositionTextureTexture*)pdata)[i].uv1);
			((D3DVertexPositionTextureTexture*)pout)[i].uv2 = toD3DFloat2(((VertexPositionTextureTexture*)pdata)[i].uv2);
			break;
		}
	}

	return pout;
}

void DxObject::loadVertexBuffer(const void* pdata, unsigned int count, VDTYPE vdType)
{
	if (pdata == nullptr || count == 0)
		throw std::invalid_argument("(DxObject::LoadVertexBuffer) Invalid vertex data");

	_vertexStride = 0;
	_vertexOffset = 0;

	switch (vdType)
	{
	case VDTYPE_POSITION:					_vertexStride = sizeof(D3DVertexPosition); break;
	case VDTYPE_POSITION_COLOR:				_vertexStride = sizeof(D3DVertexPositionColor); break;
	case VDTYPE_POSITION_COLOR_TEXTURE:		_vertexStride = sizeof(D3DVertexPositionColorTexture); break;
	case VDTYPE_POSITION_NORMAL:			_vertexStride = sizeof(D3DVertexPositionNormal); break;
	case VDTYPE_POSITION_NORMAL_TEXTURE:	_vertexStride = sizeof(D3DVertexPositionNormalTexture); break;
	case VDTYPE_POSITION_TEXTURE:			_vertexStride = sizeof(D3DVertexPositionTexture); break;
	case VDTYPE_POSITION_TEXTURE_TEXTURE:	_vertexStride = sizeof(D3DVertexPositionTextureTexture); break;
	}
	if (_vertexStride == 0)
		throw std::invalid_argument("(DxObject::LoadVertexBuffer) Invalid vertex data type");

	unsigned int size = count*_vertexStride;
	CD3D11_BUFFER_DESC vertexBufferDesc(size, D3D11_BIND_VERTEX_BUFFER);

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = toD3DVertexData(pdata, vdType, count, _vertexStride);
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	DxRender* pdr = (DxRender*)MigUtil::theRend;
	HRESULT hres = pdr->GetD3DDevice()->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&_vertexBuffer
		);
	if (hres != S_OK)
		throw hres_error("(DxObject::loadVertexBuffer) Could not create vertex buffer", hres);

	delete vertexBufferData.pSysMem;
}

void DxObject::loadIndexBuffer(const unsigned short* indices, unsigned int count, PRIMITIVE_TYPE type)
{
	if (indices == nullptr || count == 0)
		throw std::runtime_error("(DxObject::LoadIndexBuffer) Invalid vertex data");

	_indexOffset = 0;
	_indexCount = _indexOffsetCount = count;
	_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

	switch (type)
	{
	case PRIMITIVE_TYPE_TRIANGLE_LIST: _topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	case PRIMITIVE_TYPE_TRIANGLE_STRIP: _topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
	}
	if (_topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
		throw std::runtime_error("(DxObject::LoadIndexBuffer) Invalid primitive type");

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	unsigned int sizeOfData = count*sizeof(unsigned short);
	CD3D11_BUFFER_DESC indexBufferDesc(sizeOfData, D3D11_BIND_INDEX_BUFFER);

	DxRender* pdr = (DxRender*)MigUtil::theRend;
	HRESULT hres = pdr->GetD3DDevice()->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&_indexBuffer
		);
	if (hres != S_OK)
		throw hres_error("(DxObject::loadIndexBuffer) Could not create index buffer", hres);
}

void DxObject::setIndexOffset(unsigned int offset, unsigned int count)
{
	_indexOffset = offset;
	_indexOffsetCount = count;
}

int DxObject::getIndexOffset() const
{
	return _indexOffset;
}

int DxObject::getIndexCount() const
{
	return _indexOffsetCount;
}

void DxObject::prepareRender(int shaderSet)
{
	DxRender* pdr = (DxRender*)MigUtil::theRend;
	ID3D11DeviceContext1* d3dContext = pdr->GetD3DDeviceContext();

	// check to be sure the requested shader set has been loaded
	if (shaderSet < 0 || shaderSet >= (int)_shaderSets.size())
		throw std::out_of_range("(DxObject::prepareRender) Invalid shader set");
	const ShaderSet& shaderItem = _shaderSets[shaderSet];

	// set the vertex buffer
	d3dContext->IASetVertexBuffers(
		0,
		1,
		_vertexBuffer.GetAddressOf(),
		&_vertexStride,
		&_vertexOffset
		);

	// set the index buffer
	d3dContext->IASetIndexBuffer(
		_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	// set the primitive topology
	d3dContext->IASetPrimitiveTopology(_topology);

	// set the input layout for the vertex shader
	d3dContext->IASetInputLayout(shaderItem.vertexShader->getInputLayout());

	// attach our vertex shader
	d3dContext->VSSetShader(
		shaderItem.vertexShader->getVertexShader(),
		nullptr,
		0
		);

	// attach our pixel shader
	d3dContext->PSSetShader(
		shaderItem.pixelShader->getPixelShader(),
		nullptr,
		0
		);

	// set textures
	for (int i = 0; i < MAX_TEXTURE_MAPS; i++)
	{
		if (_mappings[i].pimg != nullptr && _mappings[i].pstate != nullptr)
		{
			ID3D11ShaderResourceView* pview = _mappings[i].pimg->getShaderResourceView();
			d3dContext->PSSetShaderResources(i, 1, &pview);
			d3dContext->PSSetSamplers(i, 1, &_mappings[i].pstate);
		}
	}

	// culling
	pdr->setFaceCulling(_cull);
}

void DxObject::render(int shaderSet)
{
	// if we're not in a render sequence then prepare the render
	if (!_inRenderSet)
		prepareRender(shaderSet);
	
	DxRender* pdr = (DxRender*) MigUtil::theRend;
	ID3D11DeviceContext1* d3dContext = pdr->GetD3DDeviceContext();

	// send the constant buffers to the shader
	const ShaderSet& shaderItem = _shaderSets[shaderSet];
	pdr->SendConstantBuffersToShaders(shaderItem.vertexShader->getHints(), shaderItem.pixelShader->getHints());

	// draw the objects
	d3dContext->DrawIndexed(
		_indexOffsetCount,
		_indexOffset,
		0
		);
}

void DxObject::startRenderSet(int shaderSet)
{
	prepareRender(shaderSet);

	_inRenderSet = true;
}

void DxObject::stopRenderSet()
{
	_inRenderSet = false;
}
