#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Image.h"

namespace MigTech
{
	// DirectX version of a MigTech image map
	class DxImage : public Image
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture2D;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _textureView;

	public:
		DxImage();

		void loadTexture(IMG_FORMAT fmt, int width, int height, void* pData);
		ID3D11ShaderResourceView* getShaderResourceView();
	};

	// DirectX version of a render target
	class DxRenderTarget : public DxImage
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView;
		D3D11_VIEWPORT _viewPort;

	public:
		DxRenderTarget();

		bool init(IMG_FORMAT fmtHint, int width, int height, int depthBitsHint);
		ID3D11RenderTargetView* getRenderTargetView();
		ID3D11DepthStencilView* getDepthStencilView();
		D3D11_VIEWPORT* getViewPort();
	};
}
