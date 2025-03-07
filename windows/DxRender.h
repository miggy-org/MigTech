#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/RenderBase.h"
#include "DxShader.h"
#include "DxMatrix.h"
#include "DxImage.h"

namespace MigTech
{
	// Constant buffer used to send basic information to the shaders
	struct ShaderConstantBufferBasic
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 misc;
		DirectX::XMINT4 config;
	};

	// Constant buffer used to send matrices to the shaders
	struct ShaderConstantBufferMatrix
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 proj;
		DirectX::XMFLOAT4X4 mvp;
	};

	// Constant buffer used to send lighting information to the shaders
	struct ShaderConstantBufferLights
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMFLOAT4 litColor[4];
		DirectX::XMFLOAT4 litDirPos[4];
	};

	// Windows DirectX version of the MigTech renderer
	class DxRender : public RenderBase
	{
	protected:
		virtual void createDeviceIndependentResources();
		virtual void createDeviceResources();
		virtual void createWindowSizeDependentResources();

	public:
		DxRender();
		virtual ~DxRender();

		virtual bool initRenderer();
		virtual void termRenderer();

		virtual IMatrix* createMatrix();
		virtual void deleteMatrix(IMatrix* pmat);
		virtual void setProjectionMatrix(const IMatrix* pmat);
		virtual void setProjectionMatrix(float angleY, float aspect, float nearZ, float farZ, bool useOrientation);
		virtual void setViewMatrix(const IMatrix* pmat);
		virtual void setViewMatrix(Vector3 eyePos, Vector3 focusPos, Vector3 upVector);
		virtual void setModelMatrix(const IMatrix* pmat);

		virtual Shader* loadVertexShader(const std::string& name, VDTYPE vdType, unsigned int shaderHints);
		virtual Shader* loadPixelShader(const std::string& name, unsigned int shaderHints);
		virtual Shader* getShader(const std::string& name);

		virtual Image* loadImage(const std::string& name, const std::string& path, unsigned int loadFlags);
		virtual Image* getImage(const std::string& name);
		virtual Image* createRenderTarget(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint);
		virtual void unloadImage(const std::string& name);

		virtual Object* createObject();
		virtual void deleteObject(Object* pobj);

		virtual void setOutputSize(Size newSize);
		virtual Size getOutputSize();
		virtual void setViewport(const Rect* newPort, bool clearRenderBuffer, bool clearDepthBuffer);

		virtual void setClearColor(const Color& clearCol);
		virtual void setObjectColor(const Color& objCol);
		virtual void setBlending(BLEND_STATE blend);
		virtual void setDepthTesting(DEPTH_TEST_STATE depth, bool enableWrite);
		virtual void setFaceCulling(FACE_CULLING cull);

		virtual void setMiscValue(int index, float value);
		virtual void setAmbientColor(const Color& ambientCol);
		virtual void setLightColor(int index, const Color& litCol);
		virtual void setLightDirPos(int index, const Vector3& litDirPos, bool isDir);

		virtual void onSuspending();
		virtual void onResuming();

		virtual void preRender(int pass, RenderPass* passObj);
		virtual void postRender(int pass, RenderPass* passObj);
		virtual void present();

	public:
		// Windows specific
#ifdef _WINDOWS
		void SetWindow(HWND window);
#else
		void SetWindow(Windows::UI::Core::CoreWindow^ window);
#endif // _WINDOWS
		void SetLogicalSize(Size logicalSize);
		void SetDpi(float dpi);
#ifndef _WINDOWS
		void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
#endif  // !_WINDOWS
		void SendConstantBuffersToShaders(unsigned int vertexShaderHints, unsigned int pixelShaderHints);

		// D3D Accessors
		ID3D11Device1*			GetD3DDevice() const					{ return m_d3dDevice.Get(); }
		ID3D11DeviceContext1*	GetD3DDeviceContext() const				{ return m_d3dContext.Get(); }

	protected:
		DXGI_MODE_ROTATION ComputeDisplayRotation();

	protected:
		// Cached reference to the Window.
#ifdef _WINDOWS
		HWND											m_window;
#else
		Platform::Agile<Windows::UI::Core::CoreWindow>	m_window;
#endif // _WINDOWS

		// Direct3D objects.
		Microsoft::WRL::ComPtr<ID3D11Device1>			m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1>	m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_swapChain;

		// Direct3D rendering objects. Required for 3D.
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_d3dDepthStencilStates[16];// corresponds to all supported 2*DEPTH_TEST_STATE
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_d3dRasterizerStates[3];	// corresponds to all supported FACE_CULLING
		Microsoft::WRL::ComPtr<ID3D11BlendState>		m_d3dBlendStates[5];		// corresponds to all supported BLEND_STATE
		D3D11_VIEWPORT									m_screenViewport;

		// Direct2D drawing components.
		Microsoft::WRL::ComPtr<ID2D1Factory1>		m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device>			m_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext>	m_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>		m_d2dTargetBitmap;

		// Cached device properties.
		D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
		Size											m_outputSize;
		Size											m_logicalSize;
		float											m_dpi;
#ifndef _WINDOWS
		Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
		Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
#endif  // !_WINDOWS

		// Clear color
		FLOAT m_clearColor[4];

		// System resources for geometry.
		ShaderConstantBufferBasic				m_cbBasicData;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_cbBasic;
		ShaderConstantBufferMatrix				m_cbMatrixData;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_cbMatrix;
		ShaderConstantBufferLights				m_cbLightsData;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	m_cbLights;
	
		// Shader list
		std::map<std::string, DxShader*>	m_shaders;

		// Image map list
		std::map<std::string, DxImage*> _images;

		// Matrices
		DxMatrix* m_pmatProj;
		DxMatrix* m_pmatView;
		DxMatrix* m_pmatModel;

		bool m_basicChanged;
		bool m_projChanged;
		bool m_viewChanged;
		bool m_modelChanged;
		bool m_mvpChanged;
		bool m_lightsChanged;
	};
}