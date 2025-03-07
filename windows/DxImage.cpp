#include "pch.h"
#include "../core/MigUtil.h"
#include "DxDefines.h"
#include "DxImage.h"
#include "DxRender.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

DxImage::DxImage()
{
}

void DxImage::loadTexture(IMG_FORMAT fmt, int width, int height, void* pData)
{
	DxRender* pdr = (DxRender*)MigUtil::theRend;
	ID3D11Device1* d3dDevice = pdr->GetD3DDevice();

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	switch (fmt)
	{
	case IMG_FORMAT_GREYSCALE: desc.Format = DXGI_FORMAT_R8_UNORM; break;
	case IMG_FORMAT_ALPHA: desc.Format = DXGI_FORMAT_A8_UNORM; break;
	case IMG_FORMAT_RGB: case IMG_FORMAT_RGBA: desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
	}
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA srData;
	srData.pSysMem = pData;
	srData.SysMemPitch = width*((fmt == IMG_FORMAT_GREYSCALE || fmt == IMG_FORMAT_ALPHA) ? 1 : 4);
	srData.SysMemSlicePitch = 0;

	HRESULT hres = d3dDevice->CreateTexture2D(&desc, &srData, &_texture2D);
	if (hres != S_OK)
		throw hres_error("(DxImage::loadTexture) Could not create texture", hres);

	hres = d3dDevice->CreateShaderResourceView(_texture2D.Get(), nullptr, &_textureView);
	if (hres != S_OK)
		throw hres_error("(DxImage::loadTexture) Could not create shader resource view", hres);

	_width = width;
	_height = height;
}

ID3D11ShaderResourceView* DxImage::getShaderResourceView()
{
	return _textureView.Get();
}

DxRenderTarget::DxRenderTarget()
{
	_caps = IMAGE_CAPS_RENDER_TARGET | IMAGE_CAPS_TOP_DOWN;
}

bool DxRenderTarget::init(IMG_FORMAT fmtHint, int width, int height, int depthBitsHint)
{
	DxRender* pdr = (DxRender*)MigUtil::theRend;
	ID3D11Device1* d3dDevice = pdr->GetD3DDevice();

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	switch (fmtHint)
	{
	case IMG_FORMAT_GREYSCALE: desc.Format = DXGI_FORMAT_R8_UNORM; break;
	case IMG_FORMAT_ALPHA: desc.Format = DXGI_FORMAT_A8_UNORM; break;
	case IMG_FORMAT_RGB: case IMG_FORMAT_RGBA: desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
	}
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hres = d3dDevice->CreateTexture2D(&desc, nullptr, &_texture2D);
	if (hres != S_OK && desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		LOGWARN("(DxRenderTarget::init) Cannot create single channel render target, will try 4 channels");
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		hres = d3dDevice->CreateTexture2D(&desc, nullptr, &_texture2D);
	}

	if (hres != S_OK)
	{
		LOGWARN("(DxRenderTarget::init) CreateTexture2D() returned %d", hres);
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = desc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hres = d3dDevice->CreateRenderTargetView(_texture2D.Get(), &renderTargetViewDesc, &_renderTargetView);
	if (hres != S_OK)
	{
		LOGWARN("(DxRenderTarget::init) CreateRenderTargetView() returned %d", hres);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	hres = d3dDevice->CreateShaderResourceView(_texture2D.Get(), &shaderResourceViewDesc, &_textureView);
	if (hres != S_OK)
	{
		LOGWARN("(DxRenderTarget::init) Could not create shader resource view");
		return false;
	}

	_width = width;
	_height = height;
	_viewPort = CD3D11_VIEWPORT(0.0f, 0.0f, (FLOAT)width, (FLOAT)height);
	return true;
}

ID3D11RenderTargetView* DxRenderTarget::getRenderTargetView()
{
	return _renderTargetView.Get();
}

ID3D11DepthStencilView* DxRenderTarget::getDepthStencilView()
{
	// TODO: implement separate depth stencil state for render targets when needed
	return nullptr;
}

D3D11_VIEWPORT* DxRenderTarget::getViewPort()
{
	return &_viewPort;
}
