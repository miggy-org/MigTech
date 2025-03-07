#include "pch.h"
#include "../core/MigUtil.h"
#include "DxDefines.h"
#include "DxRender.h"
#include "DxMatrix.h"
#include "DxShader.h"
#include "DxObject.h"

extern "C" {
#include "../core/libjpeg/jpeglib.h"
}

extern "C" {
#include "../core/libpng/png.h"
}

///////////////////////////////////////////////////////////////////////////
// platform specific

//using namespace D2D1;
//using namespace DirectX;
using namespace Microsoft::WRL;
//using namespace Windows::Foundation;
#ifndef _WINDOWS
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
#endif  // !_WINDOWS
//using namespace Windows::UI::Xaml::Controls;
//using namespace Platform;

using namespace MigTech;

// defined in Platform.cpp
extern bool toWString(const std::string& inStr, std::wstring& outStr);
extern const std::string& plat_getContentDir(int index);
extern const std::string& plat_getShaderDir(int index);

#if defined(_DEBUG)
// Check for SDK Layer support.
static bool SdkLayersAvailable()
{
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
		0,
		D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
		nullptr,                    // Any feature level will do.
		0,
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		nullptr,                    // No need to keep the D3D device reference.
		nullptr,                    // No need to know the feature level.
		nullptr                     // No need to keep the D3D device context reference.
		);

	return SUCCEEDED(hr);
}
#endif

// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
static float ConvertDipsToPixels(float dips, float dpi)
{
	static const float dipsPerInch = 96.0f;
	return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// Function that reads from a binary file asynchronously
//static Concurrency::task<std::vector<byte>> ReadDataAsync(const std::string& filename)
//{
//	using namespace Windows::Storage;
//	using namespace Concurrency;
//
//	auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
//
//	std::wstring temp;
//	toWString(filename, temp);
//	return create_task(folder->GetFileAsync(Platform::StringReference(temp.c_str()))).then([](StorageFile^ file)
//	{
//		return FileIO::ReadBufferAsync(file);
//	}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
//	{
//		std::vector<byte> returnBuffer;
//		returnBuffer.resize(fileBuffer->Length);
//		Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
//		return returnBuffer;
//	});
//}

// Function that reads from a binary file synchronously
static bool ReadDataSync(const std::string& filename, std::vector<byte>& retBuffer)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (!GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &data))
	{
		LOGWARN("(::ReadDataSync) File '%s' doesn't exist", filename.c_str());
		return false;
	}

	FILE* pFile = nullptr;
	if (fopen_s(&pFile, filename.c_str(), "rb"))
	{
		LOGWARN("(::ReadDataSync) Could not open file '%s'", filename.c_str());
		return false;
	}

	retBuffer.resize(data.nFileSizeLow);
	size_t sizeRead = fread_s(retBuffer.data(), retBuffer.size(), 1, retBuffer.size(), pFile);
	fclose(pFile);

	return (sizeRead == retBuffer.size());
}

DxRender::DxRender() :
	m_screenViewport(),
	m_outputSize(),
	m_logicalSize(),
	m_dpi(96.0f),
#ifndef _WINDOWS
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
#endif  // !_WINDOWS
	m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1)
{
	m_clearColor[0] = m_clearColor[1] = m_clearColor[2] = m_clearColor[3] = 0;

	m_pmatModel = new DxMatrix();
	m_pmatView = new DxMatrix();
	m_pmatProj = new DxMatrix();

	m_basicChanged = false;
	m_modelChanged = false;
	m_viewChanged = false;
	m_projChanged = false;
	m_mvpChanged = false;
	m_lightsChanged = false;
}

DxRender::~DxRender()
{
	delete m_pmatModel;
	delete m_pmatView;
	delete m_pmatProj;
}

void DxRender::createDeviceIndependentResources()
{
	HRESULT hr;

	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory1),
		&options,
		&m_d2dFactory
		);
	if (FAILED(hr))
		throw hres_error("DxRender: D2D1CreateFactory failed", hr);
}

void DxRender::createDeviceResources()
{
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (SdkLayersAvailable())
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
		0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,				// Set debug and Direct2D compatibility flags.
		featureLevels,				// List of feature levels this app can support.
		ARRAYSIZE(featureLevels),	// Size of the list above.
		D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,					// Returns the Direct3D device created.
		&m_d3dFeatureLevel,			// Returns feature level of device created.
		&context					// Returns the device immediate context.
		);

	if (FAILED(hr))
	{
		LOGWARN("(DxRender::createDeviceResources) D3D11CreateDevice() failed, trying WARP device");

		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
			0,
			creationFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&device,
			&m_d3dFeatureLevel,
			&context
			);
		if (FAILED(hr))
			throw hres_error("(DxRender::createDeviceResources) D3D11CreateDevice() failed", hr);
	}

	// Store pointers to the Direct3D 11.1 API device and immediate context.
	hr = device.As(&m_d3dDevice);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) m_d3dDevice failed", hr);

	hr = context.As(&m_d3dContext);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) m_d3dContext failed", hr);

	// Create the Direct2D device object and a corresponding context.
	ComPtr<IDXGIDevice2> dxgiDevice;
	hr = m_d3dDevice.As(&dxgiDevice);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) dxgiDevice failed", hr);

	hr = m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) m_d2dDevice failed", hr);

	hr = m_d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_d2dContext
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) CreateDeviceContext failed", hr);

	// create the constant buffers
	CD3D11_BUFFER_DESC cbBasicDesc(sizeof(ShaderConstantBufferBasic), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3dDevice->CreateBuffer(
		&cbBasicDesc,
		nullptr,
		&m_cbBasic
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) CreateBuffer failed", hr);
	CD3D11_BUFFER_DESC cbMatrixDesc(sizeof(ShaderConstantBufferMatrix), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3dDevice->CreateBuffer(
		&cbMatrixDesc,
		nullptr,
		&m_cbMatrix
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) CreateBuffer failed", hr);
	CD3D11_BUFFER_DESC cbLightsDesc(sizeof(ShaderConstantBufferLights), D3D11_BIND_CONSTANT_BUFFER);
	hr = m_d3dDevice->CreateBuffer(
		&cbLightsDesc,
		nullptr,
		&m_cbLights
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) CreateBuffer failed", hr);
}

void DxRender::createWindowSizeDependentResources()
{
	// Calculate the necessary render target size in pixels.
	float width = ConvertDipsToPixels(m_logicalSize.width, m_dpi);
	float height = ConvertDipsToPixels(m_logicalSize.height, m_dpi);

	// Prevent zero size DirectX content from being created.
	setOutputSize(MigTech::Size(max(width, 1), max(height, 1)));

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	// Windows Phone does not support resizing the swap chain, so clear it instead of resizing.
	m_swapChain = nullptr;
#endif
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d3dRenderTargetView = nullptr;
	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
	m_d3dDepthStencilView = nullptr;
	m_d3dContext->Flush();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	Size d3dRenderTargetSize;
	bool swapDimensions = false;
#ifndef _WINDOWS
#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
	swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
#endif  // !WINAPI_FAMILY_PHONE_APP
#endif  // !_WINDOWS
	d3dRenderTargetSize.width = swapDimensions ? m_outputSize.height : m_outputSize.width;
	d3dRenderTargetSize.height = swapDimensions ? m_outputSize.width : m_outputSize.height;

	HRESULT hr;
	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		hr = m_swapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			lround(d3dRenderTargetSize.width),
			lround(d3dRenderTargetSize.height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
			);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) ResizeBuffers failed", hr);
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

		swapChainDesc.Width = lround(d3dRenderTargetSize.width); // Match the size of the window.
		swapChainDesc.Height = lround(d3dRenderTargetSize.height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#ifndef _WINDOWS
		swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
#else
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // Required for Windows 7
#endif  // _WINDOWS
		swapChainDesc.Flags = 0;
#ifndef _WINDOWS
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
#endif  // _WINDOWS
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		ComPtr<IDXGIDevice2> dxgiDevice;
		hr = m_d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) dxgiDevice failed", hr);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) dxgiAdapter failed", hr);

		ComPtr<IDXGIFactory2> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) GetParent failed", hr);

#ifndef _WINDOWS
		hr = dxgiFactory->CreateSwapChainForCoreWindow(
			m_d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(m_window.Get()),
			&swapChainDesc,
			nullptr,
			&m_swapChain
			);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) CreateSwapChainForCoreWindow failed", hr);
#else
		hr = dxgiFactory->CreateSwapChainForHwnd(
			m_d3dDevice.Get(),
			m_window,
			&swapChainDesc,
			nullptr,
			nullptr,
			&m_swapChain
			);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) CreateSwapChainForHwnd failed", hr);

		// We don't handle full-screen swapchains so we block the ALT+ENTER shortcut
		dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER);
#endif  // !_WINDOWS

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
		// ensures that the application will only render after each VSync, minimizing power consumption.
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr))
			throw hres_error("(DxRender::createWindowSizeDependentResources) SetMaximumFrameLatency failed", hr);
	}

#ifndef _WINDOWS
	hr = m_swapChain->SetRotation(displayRotation);
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) SetRotation failed", hr);
#endif  // !_WINDOWS

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) GetBuffer failed", hr);

	hr = m_d3dDevice->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		&m_d3dRenderTargetView
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) CreateRenderTargetView failed", hr);

	// Create a depth stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		lround(d3dRenderTargetSize.width),
		lround(d3dRenderTargetSize.height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D> depthStencil;
	hr = m_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencil
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) CreateTexture2D failed", hr);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	hr = m_d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		&m_d3dDepthStencilView
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) CreateDepthStencilView failed", hr);

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		d3dRenderTargetSize.width,
		d3dRenderTargetSize.height
		);

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
		m_dpi,
		m_dpi
		);

	ComPtr<IDXGISurface2> dxgiBackBuffer;
	hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) GetBuffer failed", hr);

	hr = m_d2dContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&bitmapProperties,
		&m_d2dTargetBitmap
		);
	if (FAILED(hr))
		throw hres_error("(DxRender::createWindowSizeDependentResources) CreateBitmapFromDxgiSurface failed", hr);

	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

	// Grayscale text anti-aliasing is recommended for all Windows Store apps.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	// defaults
	setDepthTesting(DEPTH_TEST_STATE_NONE, false);
	setFaceCulling(FACE_CULLING_BACK);
	setBlending(BLEND_STATE_NONE);
}

bool DxRender::initRenderer()
{
	createDeviceIndependentResources();
	createDeviceResources();

	return true;
}

void DxRender::termRenderer()
{
	m_cbBasic.Reset();
	m_cbMatrix.Reset();
	m_cbLights.Reset();

	std::map<std::string, DxShader*>::const_iterator iter;
	for (iter = m_shaders.begin(); iter != m_shaders.end(); iter++)
	{
		delete iter->second;
	}
	m_shaders.clear();
}

IMatrix* DxRender::createMatrix()
{
	return (IMatrix*) new DxMatrix();
}

void DxRender::deleteMatrix(IMatrix* pmat)
{
	delete (DxMatrix*)pmat;
}

void DxRender::setProjectionMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
		m_pmatProj->copy((DxMatrix*)pmat);
	else
		m_pmatProj->identity();
	m_projChanged = m_mvpChanged = true;
}

void DxRender::setProjectionMatrix(float angleY, float aspect, float nearZ, float farZ, bool useOrientation)
{
	DxMatrix dxmat;
	dxmat.loadPerspectiveFovRH(angleY, aspect, nearZ, farZ);

	if (useOrientation)
	{
		DxMatrix dxomat;

		DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();
		switch (displayRotation)
		{
		case DXGI_MODE_ROTATION_ROTATE90:
			// 270-degree Z-rotation
			dxomat.load(
				0.0f, -1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DXGI_MODE_ROTATION_ROTATE180:
			// 180-degree Z-rotation
			dxomat.load(
				-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DXGI_MODE_ROTATION_ROTATE270:
			// 90-degree Z-rotation
			dxomat.load(
				0.0f, 1.0f, 0.0f, 0.0f,
				-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DXGI_MODE_ROTATION_IDENTITY:
		default:
			// 0-degree Z-rotation
			dxomat.identity();
			break;
		}

		dxmat.multiply(&dxomat);
	}

	setProjectionMatrix(&dxmat);
}

void DxRender::setViewMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
		m_pmatView->copy((DxMatrix*)pmat);
	else
		m_pmatView->identity();
	m_viewChanged = m_mvpChanged = true;
}

void DxRender::setViewMatrix(Vector3 eyePos, Vector3 focusPos, Vector3 upVector)
{
	DxMatrix dxmat;
	dxmat.loadLookAtRH(eyePos, focusPos, upVector);
	setViewMatrix(&dxmat);
}

void DxRender::setModelMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
		m_pmatModel->copy((DxMatrix*)pmat);
	else
		m_pmatModel->identity();
	m_modelChanged = m_mvpChanged = true;
}

void DxRender::setOutputSize(Size newSize)
{
	m_outputSize = newSize;
}

Size DxRender::getOutputSize()
{
	return m_outputSize;
}

static const D3D11_VIEWPORT* toD3DViewport(const Rect& newPort, float width, float height)
{
	static D3D11_VIEWPORT vp;
	vp = CD3D11_VIEWPORT(
		newPort.corner.x * width,
		newPort.corner.y * height,
		newPort.size.width * width,
		newPort.size.height * height
		);
	return &vp;
}

void DxRender::setViewport(const Rect* newPort, bool clearRenderBuffer, bool clearDepthBuffer)
{
	// Reset the viewport
	if (newPort != nullptr)
		m_d3dContext->RSSetViewports(1, toD3DViewport(*newPort, m_screenViewport.Width, m_screenViewport.Height));
	else
		m_d3dContext->RSSetViewports(1, &m_screenViewport);

	// Clear the back buffer and depth stencil view
	if (clearRenderBuffer)
		m_d3dContext->ClearRenderTargetView(m_d3dRenderTargetView.Get(), m_clearColor);
	if (clearDepthBuffer)
		m_d3dContext->ClearDepthStencilView(m_d3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

#ifdef _WINDOWS
static HRESULT CompileShaderFromFile(const std::string& fileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// compose the full path
	std::wstring fullPath;
	toWString(plat_getShaderDir(0) + fileName + ".hlsl", fullPath);
	if (::GetFileAttributes(fullPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		toWString(plat_getShaderDir(1) + fileName + ".hlsl", fullPath);

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(fullPath.c_str(), nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			LOGERR("(::CompileShaderFromFile) %s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}
#endif // _WINDOWS

/*D3D11_INPUT_ELEMENT_DESC* toD3DInputLayout(const SHADER_INPUT_DESC* layout, int elemSize)
{
	D3D11_INPUT_ELEMENT_DESC* desc = new D3D11_INPUT_ELEMENT_DESC[elemSize];
	for (int i = 0; i < elemSize; i++)
	{
		desc[i].SemanticName = layout[i].name;
		desc[i].SemanticIndex = layout[i].index;
		desc[i].InputSlot = layout[i].slot;
		desc[i].AlignedByteOffset = layout[i].offset;
		desc[i].InputSlotClass = (layout[i].inputClass == INPUT_CLASS_PER_VERTEX ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA);
		desc[i].InstanceDataStepRate = layout[i].instaceStepRate;

		desc[i].Format = DXGI_FORMAT_UNKNOWN;
		switch (layout[i].format)
		{
		case ELEM_FORMAT_FLOAT_4X: desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		case ELEM_FORMAT_FLOAT_3X: desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
		case ELEM_FORMAT_FLOAT_2X: desc[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
		case ELEM_FORMAT_FLOAT: desc[i].Format = DXGI_FORMAT_R32_FLOAT; break;
		}
	}
	return desc;
}*/

const D3D11_INPUT_ELEMENT_DESC* toD3DInputLayout(VDTYPE vdType, unsigned int& elemCount)
{
	const D3D11_INPUT_ELEMENT_DESC* desc = nullptr;

	if (vdType == VDTYPE_POSITION)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_COLOR)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_COLOR_TEXTURE)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_NORMAL)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_NORMAL_TEXTURE)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_TEXTURE)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}
	else if (vdType == VDTYPE_POSITION_TEXTURE_TEXTURE)
	{
		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		desc = vertexDesc;
		elemCount = ARRAYSIZE(vertexDesc);
	}

	return desc;
}

Shader* DxRender::loadVertexShader(const std::string& name, VDTYPE vdType, unsigned int shaderHints)
{
	if (vdType == VDTYPE_UNKNOWN)
		throw std::invalid_argument("(DxRender::LoadVertexShader) No input layout specified");

	// see if the shader already exists
	Shader* ps = getShader(name);
	if (ps != nullptr)
	{
		// must match requested shader type
		if (ps->getType() != Shader::SHADER_TYPE_VERTEX)
			throw std::invalid_argument("(DxRender::LoadVertexShader) Existing shader doesn't match requested shader type");
		return ps;
	}

	HRESULT hres;
#ifdef _WINDOWS
	// compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hres = CompileShaderFromFile(name, "main", "vs_4_0", &pVSBlob);
	if (hres != S_OK)
		throw hres_error("(DxRender::loadVertexShader) Could not compile shader", hres);
	const void* shaderByteCode = pVSBlob->GetBufferPointer();
	SIZE_T byteCodeLength = pVSBlob->GetBufferSize();
#else
	std::vector<byte> fileData;
	if (!ReadDataSync(name + ".cso", fileData))
		throw std::runtime_error("(DxRender::loadVertexShader) Could not load shader");
	const void* shaderByteCode = &fileData[0];
	SIZE_T byteCodeLength = fileData.size();
#endif // _WINDOWS

	ID3D11VertexShader* vertexShader;
	hres = m_d3dDevice->CreateVertexShader(
		shaderByteCode,
		byteCodeLength,
		nullptr,
		&vertexShader
		);
	if (hres != S_OK || vertexShader == nullptr)
		throw hres_error("(DxRender::loadVertexShader) Could not load shader", hres);

	unsigned int elemCount = 0;
	const D3D11_INPUT_ELEMENT_DESC* vertexDesc = toD3DInputLayout(vdType, elemCount);
	ID3D11InputLayout* inputLayout;
	hres = m_d3dDevice->CreateInputLayout(
		vertexDesc,
		elemCount,
		shaderByteCode,
		byteCodeLength,
		&inputLayout
		);
	if (hres != S_OK || inputLayout == nullptr)
		throw hres_error("(DxRender::loadVertexShader) Could not create input layout", hres);
#ifdef _WINDOWS
	pVSBlob->Release();
#endif // _WINDOWS

	DxShader* psNew = new DxShader(vertexShader, inputLayout, shaderHints);
	m_shaders[name] = psNew;
	return psNew;
}

Shader* DxRender::loadPixelShader(const std::string& name, unsigned int shaderHints)
{
	// see if the shader already exists
	Shader* ps = getShader(name);
	if (ps != nullptr)
	{
		// must match requested shader type
		if (ps->getType() != Shader::SHADER_TYPE_PIXEL)
			throw std::runtime_error("(DxRender::loadPixelShader) Existing shader doesn't match requested shader type");
		return ps;
	}

	HRESULT hres;
#ifdef _WINDOWS
	// compile the shader
	ID3DBlob* pPSBlob = nullptr;
	hres = CompileShaderFromFile(name, "main", "ps_4_0", &pPSBlob);
	if (hres != S_OK)
		throw hres_error("(DxRender::loadPixelShader) Could not compile shader", hres);
	const void* shaderByteCode = pPSBlob->GetBufferPointer();
	SIZE_T byteCodeLength = pPSBlob->GetBufferSize();
#else
	std::vector<byte> fileData;
	if (!ReadDataSync(name + ".cso", fileData))
		throw std::runtime_error("(DxRender::loadPixelShader) Could not load shader");
	const void* shaderByteCode = &fileData[0];
	SIZE_T byteCodeLength = fileData.size();
#endif // _WINDOWS

	ID3D11PixelShader* pixelShader;
	hres = m_d3dDevice->CreatePixelShader(
		shaderByteCode,
		byteCodeLength,
		nullptr,
		&pixelShader
		);
	if (hres != S_OK || pixelShader == nullptr)
		throw hres_error("(DxRender::loadPixelShader) Could not load shader", hres);
#ifdef _WINDOWS
	pPSBlob->Release();
#endif // _WINDOWS

	DxShader* psNew = new DxShader(pixelShader, shaderHints);
	m_shaders[name] = psNew;
	return psNew;
}

Shader* DxRender::getShader(const std::string& name)
{
	std::map<std::string, DxShader*>::const_iterator iter = m_shaders.find(name);
	if (iter != m_shaders.end() && iter->second != nullptr)
		return iter->second;
	return nullptr;
}

/*void DxRender::SetVertexShader(ShaderBase* vertexShader)
{
	if (vertexShader == nullptr || vertexShader->GetType() != ShaderBase::Type::SHADER_TYPE_VERTEX)
		throw std::runtime_error("(DxRender::SetVertexShader) Shader argument isn't a vertex shader");
	DxShader* pdxs = (DxShader*)vertexShader;

	// Set the input layout for the vertex shader
	m_d3dContext->IASetInputLayout(pdxs->getInputLayout());

	// Attach our vertex shader.
	m_d3dContext->VSSetShader(
		pdxs->getVertexShader(),
		nullptr,
		0
		);
}*/

/*void DxRender::SetPixelShader(ShaderBase* pixelShader)
{
	if (pixelShader == nullptr || pixelShader->GetType() != ShaderBase::Type::SHADER_TYPE_PIXEL)
		throw std::runtime_error("(DxRender::SetPixelShader) Shader argument isn't a pixel shader");
	DxShader* pdxs = (DxShader*)pixelShader;

	// Attach our pixel shader.
	m_d3dContext->PSSetShader(
		pdxs->getPixelShader(),
		nullptr,
		0
		);
}*/

static DxImage* loadJPEGImage(const std::string& name, unsigned int loadFlags)
{
	// compose the complete path to the asset and open it
	std::string path = plat_getContentDir(0) + name;
	FILE* pFile = nullptr;
	if (fopen_s(&pFile, path.c_str(), "rb"))
	{
		LOGWARN("(DxRender::loadJPEGImage) image '%s' doesn't exist", name.c_str());
		return nullptr;
	}

	// initialize decompression
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	LOGINFO("(DxRender::loadJPEGImage) Image=%s, w=%d, h=%d", name.c_str(), cinfo.output_width, cinfo.output_height);

	// for now, only 24-bit JPEGs
	byte* pData = nullptr;
	if (cinfo.output_components == 3)
	{
		// read into the image buffer (note that DxImage only supports 4 byte per pixel images)
		int row_stride = 4 * cinfo.output_width;
		int numPixels = row_stride * cinfo.output_height;
		pData = new byte[numPixels];
		if (pData != nullptr)
		{
			byte* psrc = new byte[3 * cinfo.output_width];
			JSAMPROW row_pointer[1];
			row_pointer[0] = psrc;

			while (cinfo.output_scanline < cinfo.output_height)
			{
				jpeg_read_scanlines(&cinfo, row_pointer, 1);

				byte* pdst = pData + (cinfo.output_scanline - 1)*row_stride;
				for (unsigned int i = 0; i < cinfo.output_width; i++)
				{
					pdst[4 * i + 0] = psrc[3 * i + 0];
					pdst[4 * i + 1] = psrc[3 * i + 1];
					pdst[4 * i + 2] = psrc[3 * i + 2];

					// apply image processing
					byte alpha = 255;
					if (loadFlags != LOAD_IMAGE_NONE)
					{
						if (loadFlags & LOAD_IMAGE_ADD_ALPHA)
							alpha = (pdst[4 * i + 0] + pdst[4 * i + 1] + pdst[4 * i + 2]) / 3;
						else if (loadFlags & LOAD_IMAGE_CLEAR_ALPHA)
							alpha = 0;
						if (loadFlags & LOAD_IMAGE_INVERT_ALPHA)
							alpha = (255 - alpha);
					}
					pdst[4 * i + 3] = alpha;
				}
			}
			delete psrc;

			// drop color components if so flagged
			if (loadFlags & LOAD_IMAGE_DROP_COLOR)
			{
				int numPixels = cinfo.output_width * cinfo.output_height;
				byte* pNewData = new byte[numPixels];
				for (int i = 0; i < numPixels; i++)
				{
					pNewData[i] = pData[4 * i + 3];
				}

				delete pData;
				pData = pNewData;
			}
		}
	}
	else
	{
		LOGWARN("(DxRender::loadJPEGImage) %d color components not supported", cinfo.output_components);
	}

	// clean up decompression
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(pFile);

	// load the data into an Image and return
	DxImage* newImage = nullptr;
	if (pData != nullptr)
	{
		// create the image object and load
		newImage = new DxImage();
		newImage->loadTexture(loadFlags & LOAD_IMAGE_DROP_COLOR ? IMG_FORMAT_ALPHA : IMG_FORMAT_RGBA, cinfo.output_width, cinfo.output_height, pData);
		delete pData;
	}
	return newImage;
}

static DxImage* loadPNGImage(const std::string& name, unsigned int loadFlags)
{
	// compose the complete path to the asset and open it
	std::string path = plat_getContentDir(0) + name;
	FILE* pFile = nullptr;
	if (fopen_s(&pFile, path.c_str(), "rb"))
	{
		LOGWARN("(DxRender::loadPNGImage) image '%s' doesn't exist", name.c_str());
		return nullptr;
	}

	// check the header to ensure it's a PNG
	byte header[8];
	fread(header, 1, sizeof(header), pFile);
	if (png_sig_cmp(header, 0, sizeof(header)))
	{
		LOGWARN("(DxRender::loadPNGImage) image '%s' does not appear to be a PNG", name.c_str());
		fclose(pFile);
		return nullptr;
	}

	// allocate needed structs
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
	{
		LOGWARN("(DxRender::loadPNGImage) png_create_read_struct() failed");
		fclose(pFile);
		return nullptr;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		LOGWARN("(DxRender::loadPNGImage) png_create_info_struct() failed");
		png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
		fclose(pFile);
		return nullptr;
	}

	// init the PNG file IO
	png_init_io(png_ptr, pFile);
	png_set_sig_bytes(png_ptr, sizeof(header));

	// read the info header
	png_read_info(png_ptr, info_ptr);

	// transform images to RGBA type (unless greyscale)
	int colorType = png_get_color_type(png_ptr, info_ptr);
	if (colorType == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
		png_set_add_alpha(png_ptr, 0, PNG_FILLER_AFTER);
	}
	else if (colorType == PNG_COLOR_TYPE_RGB)
		png_set_add_alpha(png_ptr, 0, PNG_FILLER_AFTER);

	// add alpha if the transparency is in a tRNS chunk
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// expand image data that is less than 8 bits to 8 bits
	int bitDepth = png_get_bit_depth(png_ptr, info_ptr);
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	else if (bitDepth < 8)
		png_set_packing(png_ptr);

	// apply the transformations
	png_read_update_info(png_ptr, info_ptr);

	// re-read header info
	byte* pData = nullptr;
	colorType = png_get_color_type(png_ptr, info_ptr);
	bitDepth = png_get_bit_depth(png_ptr, info_ptr);
	int channels = png_get_channels(png_ptr, info_ptr);
	int imageWidth = png_get_image_width(png_ptr, info_ptr);
	int imageHeight = png_get_image_height(png_ptr, info_ptr);
	LOGINFO("(DxRender::loadPNGImage) Image=%s, w=%d, h=%d, bits=%d, channels=%d", name.c_str(), imageWidth, imageHeight, bitDepth, channels);
	if (bitDepth == 8 && channels != 2 && channels != 3)
	{
		// allocate image buffer data
		int row_stride = (channels == 1 ? 1 : 4) * imageWidth;
		pData = new byte[row_stride*imageHeight];
		if (pData != nullptr)
		{
			// read the PNG
			png_bytepp row_pointers = new png_bytep[imageHeight];
			for (int y = 0; y < imageHeight; y++)
				row_pointers[y] = pData + y*row_stride;
			png_read_image(png_ptr, row_pointers);
			delete[] row_pointers;

			// read the end of the PNG (skip the actual data)
			png_read_end(png_ptr, (png_infop)nullptr);

			// apply image processing
			if (loadFlags != LOAD_IMAGE_NONE && channels != 1)
			{
				int numPixels = imageWidth * imageHeight;
				for (int i = 0; i < numPixels; i++)
				{
					byte alpha;
					if (loadFlags & LOAD_IMAGE_ADD_ALPHA)
						alpha = (pData[4 * i + 0] + pData[4 * i + 1] + pData[4 * i + 2]) / 3;
					else if (loadFlags & LOAD_IMAGE_SET_ALPHA)
						alpha = 255;
					else if (loadFlags & LOAD_IMAGE_CLEAR_ALPHA)
						alpha = 0;
					else
						alpha = pData[4 * i + 3];
					if (loadFlags & LOAD_IMAGE_INVERT_ALPHA)
						alpha = (255 - alpha);
					pData[4 * i + 3] = alpha;
				}
			}
		}

		// drop color components if so flagged
		if (loadFlags & LOAD_IMAGE_DROP_COLOR)
		{
			int numPixels = imageWidth * imageHeight;
			byte* pNewData = new byte[numPixels];
			for (int i = 0; i < numPixels; i++)
			{
				pNewData[i] = pData[4 * i + 3];
			}

			delete pData;
			pData = pNewData;
		}
	}
	else
	{
		LOGWARN("(DxRender::loadPNGImage) Unsupported bit depth or channels (%d, %d)", bitDepth, channels);
	}
	
	// clean up
	png_destroy_read_struct(&png_ptr, (png_infopp)&info_ptr, (png_infopp)nullptr);
	fclose(pFile);

	// load the data into an Image and return
	DxImage* newImage = nullptr;
	if (pData != nullptr)
	{
		// create the image object and load
		newImage = new DxImage();
		newImage->loadTexture(loadFlags & LOAD_IMAGE_DROP_COLOR ? IMG_FORMAT_ALPHA : IMG_FORMAT_RGBA, imageWidth, imageHeight, pData);
		delete pData;
	}
	return newImage;
}

Image* DxRender::loadImage(const std::string& name, const std::string& path, unsigned int loadFlags)
{
	// see if the image already exists
	Image* pi = getImage(name);
	if (pi != nullptr)
		return pi;

	DxImage* newImage = nullptr;
	int findDot = path.rfind(".");
	if (findDot != string::npos)
	{
		std::string ext = path.substr(findDot + 1);
		if (0 == _stricmp(ext.c_str(), "jpg") ||
			0 == _stricmp(ext.c_str(), "jpeg"))
		{
			newImage = loadJPEGImage(path, loadFlags);
		}
		else if (0 == _stricmp(ext.c_str(), "png"))
		{
			newImage = loadPNGImage(path, loadFlags);
		}
	}
	if (newImage != nullptr)
		_images[name] = newImage;

	return newImage;
}

Image* DxRender::getImage(const std::string& name)
{
	std::map<std::string, DxImage*>::const_iterator iter = _images.find(name);
	if (iter != _images.end() && iter->second != nullptr)
		return iter->second;
	return nullptr;
}

Image* DxRender::createRenderTarget(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint)
{
	// if the render target already exists, that is considered an error
	if (getImage(name) != nullptr)
		return nullptr;

	DxRenderTarget* newTarget = new DxRenderTarget();
	if (!newTarget->init(fmtHint, width, height, depthBitsHint))
	{
		delete newTarget;
		return nullptr;
	}

	_images[name] = newTarget;
	return newTarget;
}

void DxRender::unloadImage(const std::string& name)
{
	std::map<std::string, DxImage*>::iterator iter = _images.find(name);
	if (iter != _images.end() && iter->second != nullptr)
	{
		delete iter->second;
		_images.erase(iter);
	}
}

Object* DxRender::createObject()
{
	return new DxObject();
}

void DxRender::deleteObject(Object* pobj)
{
	delete (DxObject*)pobj;
}

void DxRender::setClearColor(const Color& clearCol)
{
	m_clearColor[0] = clearCol.r;
	m_clearColor[1] = clearCol.g;
	m_clearColor[2] = clearCol.b;
	m_clearColor[3] = clearCol.a;
}

void DxRender::setObjectColor(const Color& objCol)
{
	m_cbBasicData.color.x = objCol.r;
	m_cbBasicData.color.y = objCol.g;
	m_cbBasicData.color.z = objCol.b;
	m_cbBasicData.color.w = objCol.a;
	m_basicChanged = true;
}

// creates a D3D11_BLEND_DESC for a given MigTech BLEND_STATE value
static D3D11_BLEND_DESC createBlendDesc(BLEND_STATE blend)
{
	D3D11_BLEND_DESC bd;
	memset(&bd, 0, sizeof(D3D11_BLEND_DESC));
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	switch (blend)
	{
		case BLEND_STATE_NONE:
			bd.RenderTarget[0] = { FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
			break;
		case BLEND_STATE_SRC_ALPHA:
			bd.RenderTarget[0] = { TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
			break;
		case BLEND_STATE_ONE_MINUS_SRC_ALPHA:
			bd.RenderTarget[0] = { TRUE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
			break;
		case BLEND_STATE_DST_ALPHA:
			bd.RenderTarget[0] = { TRUE, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
			break;
		case BLEND_STATE_ONE_MINUS_DST_ALPHA:
			bd.RenderTarget[0] = { TRUE, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
			break;
	}
	return bd;
}

void DxRender::setBlending(BLEND_STATE blend)
{
	if (m_d3dBlendStates[blend] == nullptr)
	{
		D3D11_BLEND_DESC bd = createBlendDesc(blend);

		ID3D11BlendState* pbs = nullptr;
		m_d3dDevice->CreateBlendState(&bd, &pbs);
		m_d3dBlendStates[blend].Attach(pbs);
	}

	if (m_d3dBlendStates[blend] != nullptr)
	{
		m_d3dContext->OMSetBlendState(m_d3dBlendStates[blend].Get(), nullptr, 0xffffffff);
	}
}

// creates a D3D11_DEPTH_STENCIL_DESC for a given MigTech DEPTH_TEST_STATE value
static D3D11_DEPTH_STENCIL_DESC createDepthStencilDesc(DEPTH_TEST_STATE depth, bool enableWrite)
{
	D3D11_DEPTH_STENCIL_DESC dsd;
	dsd.DepthEnable = (depth == DEPTH_TEST_STATE_NONE ? FALSE : TRUE);
	dsd.DepthWriteMask = (enableWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
	switch (depth)
	{
	case DEPTH_TEST_STATE_LESS:		dsd.DepthFunc = D3D11_COMPARISON_LESS; break;
	case DEPTH_TEST_STATE_LEQUAL:	dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; break;
	case DEPTH_TEST_STATE_EQUAL:	dsd.DepthFunc = D3D11_COMPARISON_EQUAL; break;
	case DEPTH_TEST_STATE_GEQUAL:	dsd.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
	case DEPTH_TEST_STATE_GREATER:	dsd.DepthFunc = D3D11_COMPARISON_GREATER; break;
	case DEPTH_TEST_STATE_NEQUAL:	dsd.DepthFunc = D3D11_COMPARISON_NOT_EQUAL; break;
	default:						dsd.DepthFunc = D3D11_COMPARISON_ALWAYS; break;
	}
	dsd.StencilEnable = FALSE;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsd.FrontFace.StencilFunc = dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilDepthFailOp = dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilPassOp = dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFailOp = dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	return dsd;
}

void DxRender::setDepthTesting(DEPTH_TEST_STATE depth, bool enableWrite)
{
	int index = (int)depth + (enableWrite ? 8 : 0);
	if (m_d3dDepthStencilStates[index] == nullptr)
	{
		D3D11_DEPTH_STENCIL_DESC dsd = createDepthStencilDesc(depth, enableWrite);

		ID3D11DepthStencilState* pdss = nullptr;
		m_d3dDevice->CreateDepthStencilState(&dsd, &pdss);
		m_d3dDepthStencilStates[index].Attach(pdss);
	}

	if (m_d3dDepthStencilStates[index] != nullptr)
	{
		m_d3dContext->OMSetDepthStencilState(m_d3dDepthStencilStates[index].Get(), 0);
	}
}

// creates a D3D11_RASTERIZER_DESC for a given MigTech FACE_CULLING value
static D3D11_RASTERIZER_DESC createRasterizerDesc(FACE_CULLING cull)
{
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = (cull == FACE_CULLING_NONE ? D3D11_CULL_NONE : (cull == FACE_CULLING_FRONT ? D3D11_CULL_FRONT : D3D11_CULL_BACK));
	rd.FrontCounterClockwise = TRUE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;
	return rd;
}

void DxRender::setFaceCulling(FACE_CULLING cull)
{
	if (m_d3dRasterizerStates[cull] == nullptr)
	{
		D3D11_RASTERIZER_DESC rs = createRasterizerDesc(cull);

		ID3D11RasterizerState* prs = nullptr;
		m_d3dDevice->CreateRasterizerState(&rs, &prs);
		m_d3dRasterizerStates[cull].Attach(prs);
	}

	if (m_d3dRasterizerStates[cull] != nullptr)
	{
		m_d3dContext->RSSetState(m_d3dRasterizerStates[cull].Get());
	}
}

void DxRender::setMiscValue(int index, float value)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(DxRender::createDeviceResources) Misc index out of bounds");
	if (index == 0)
		m_cbBasicData.misc.x = value;
	else if (index == 1)
		m_cbBasicData.misc.y = value;
	else if (index == 2)
		m_cbBasicData.misc.z = value;
	else if (index == 3)
		m_cbBasicData.misc.w = value;
	m_basicChanged = true;
}

void DxRender::setAmbientColor(const Color& ambientCol)
{
	m_cbLightsData.ambientColor.x = ambientCol.r;
	m_cbLightsData.ambientColor.y = ambientCol.g;
	m_cbLightsData.ambientColor.z = ambientCol.b;
	m_cbLightsData.ambientColor.w = ambientCol.a;
	m_lightsChanged = true;
}

void DxRender::setLightColor(int index, const Color& litCol)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(DxRender::createDeviceResources) Light index out of bounds");
	m_cbLightsData.litColor[index].x = litCol.r;
	m_cbLightsData.litColor[index].y = litCol.g;
	m_cbLightsData.litColor[index].z = litCol.b;
	m_cbLightsData.litColor[index].w = litCol.a;
	m_lightsChanged = true;
}

void DxRender::setLightDirPos(int index, const Vector3& litDirPos, bool isDir)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(DxRender::createDeviceResources) Light index out of bounds");
	m_cbLightsData.litDirPos[index].x = litDirPos.x;
	m_cbLightsData.litDirPos[index].y = litDirPos.y;
	m_cbLightsData.litDirPos[index].z = litDirPos.z;
	m_cbLightsData.litDirPos[index].w = (isDir ? 0.0f : 1.0f);
	m_lightsChanged = true;
}

void DxRender::onSuspending()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	HRESULT hr = m_d3dDevice.As(&dxgiDevice);
	if (SUCCEEDED(hr))
		dxgiDevice->Trim();
}

void DxRender::onResuming()
{
}

static const FLOAT* toD3DColor(const Color& color)
{
	static FLOAT d3dColor[4];
	d3dColor[0] = color.r;
	d3dColor[1] = color.g;
	d3dColor[2] = color.b;
	d3dColor[3] = color.a;
	return d3dColor;
}

void DxRender::preRender(int pass, RenderPass* passObj)
{
	unsigned int configBits = 0;
	if (pass > 0 && passObj != nullptr)
		configBits = passObj->getConfigBits();

	// get render target
	DxRenderTarget* target = nullptr;
	if (configBits & RenderPass::USE_RENDER_TARGET)
		target = (DxRenderTarget*)passObj->getRenderTarget();

	// reset the viewport (note that render target viewport takes precedence over render pass viewport)
	const D3D11_VIEWPORT* viewPort = &m_screenViewport;
	if (target != nullptr)
		viewPort = target->getViewPort();
	else if (configBits & RenderPass::USE_VIEW_PORT)
		viewPort = toD3DViewport(passObj->getViewPort(), m_screenViewport.Width, m_screenViewport.Height);
	m_d3dContext->RSSetViewports(1, viewPort);

	// reset render targets to the screen or render target
	ID3D11RenderTargetView *const targets[1] = { (target != nullptr ? target->getRenderTargetView() : m_d3dRenderTargetView.Get()) };
	ID3D11DepthStencilView *const depth = (target != nullptr ? target->getDepthStencilView() : m_d3dDepthStencilView.Get());
	m_d3dContext->OMSetRenderTargets(1, targets, depth);

	// clear the render buffer
	if (configBits & RenderPass::USE_CLEAR_COLOR)
		m_d3dContext->ClearRenderTargetView(targets[0], toD3DColor(passObj->getClearColor()));
	else if (pass == 0)
		m_d3dContext->ClearRenderTargetView(targets[0], m_clearColor);

	// clear the depth buffer, if in use
	if (depth != nullptr)
	{
		if (pass == 0 || (configBits & RenderPass::USE_CLEAR_DEPTH))
			m_d3dContext->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	// first config int will contain the pass (1 based, 0 means main pass, -1 means overlay pass)
	m_cbBasicData.config.x = pass;
	m_basicChanged = true;
}

void DxRender::postRender(int pass, RenderPass* passObj)
{
}

// this isn't supported by MigTech yet, but may be if necessary
/*void DxRender::HandleDeviceLost()
{
	m_swapChain = nullptr;

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceLost();
	}

	CreateDeviceResources();
	m_d2dContext->SetDpi(m_dpi, m_dpi);
	CreateWindowSizeDependentResources();

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}*/

void DxRender::present()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

	// Discard the contents of the depth stencil.
	m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		// TODO: handle this case if necessary (see HandleDeviceLost)
		throw hres_error("(DxRender::createDeviceResources) D3D device removed/reset", hr);
	}
	else if (FAILED(hr))
		throw hres_error("(DxRender::createDeviceResources) Present failed", hr);
}

#ifdef _WINDOWS
// This method is called when the HWND is created
void DxRender::SetWindow(HWND window)
{
	m_window = window;

	RECT rc;
	GetClientRect(m_window, &rc);
	m_logicalSize = Size((float)(rc.right - rc.left), (float)(rc.bottom - rc.top));

	createWindowSizeDependentResources();
}
#else
// This method is called when the CoreWindow is created (or re-created).
void DxRender::SetWindow(CoreWindow^ window)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = Size(window->Bounds.Width, window->Bounds.Height);
	m_dpi = currentDisplayInformation->LogicalDpi;
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_d2dContext->SetDpi(m_dpi, m_dpi);

	createWindowSizeDependentResources();
}
#endif // _WINDOWS

// This method is called in the event handler for the SizeChanged event.
void DxRender::SetLogicalSize(Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		createWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DpiChanged event.
void DxRender::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
#ifdef _WINDOWS
		RECT rc;
		GetClientRect(m_window, &rc);
		m_logicalSize = Size((float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
#else
		m_logicalSize = Size(m_window->Bounds.Width, m_window->Bounds.Height);
#endif // _WINDOWS

		m_d2dContext->SetDpi(m_dpi, m_dpi);
		createWindowSizeDependentResources();
	}
}

#ifndef _WINDOWS
// This method is called in the event handler for the OrientationChanged event.
void DxRender::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		createWindowSizeDependentResources();
	}
}
#endif  // !_WINDOWS

// This method determines the rotation between the display device's native Orientation and the
// current display orientation.
DXGI_MODE_ROTATION DxRender::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;

#ifndef _WINDOWS
	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
#endif  // !_WINDOWS
	return rotation;
}

void DxRender::SendConstantBuffersToShaders(unsigned int vertexShaderHints, unsigned int pixelShaderHints)
{
	// currently we only support matrices being sent to vertex shaders
	bool needMatrices = false;
	if (vertexShaderHints & SHADER_HINT_MODEL)
	{
		if (m_modelChanged)
		{
			XMStoreFloat4x4(
				&m_cbMatrixData.model,
				DirectX::XMMatrixTranspose(m_pmatModel->getMat())
				);
			m_modelChanged = false;
			needMatrices = true;
		}
	}
	if (vertexShaderHints & SHADER_HINT_VIEW)
	{
		if (m_viewChanged)
		{
			XMStoreFloat4x4(
				&m_cbMatrixData.view,
				DirectX::XMMatrixTranspose(m_pmatView->getMat())
				);
			m_viewChanged = false;
			needMatrices = true;
		}
	}
	if (vertexShaderHints & SHADER_HINT_PROJ)
	{
		if (m_projChanged)
		{
			XMStoreFloat4x4(
				&m_cbMatrixData.proj,
				DirectX::XMMatrixTranspose(m_pmatProj->getMat())
				);
			m_projChanged = false;
			needMatrices = true;
		}
	}
	if (vertexShaderHints & SHADER_HINT_MVP)
	{
		if (m_mvpChanged)
		{
			DirectX::XMMATRIX mvp = m_pmatModel->getMat();
			mvp = XMMatrixMultiply(mvp, m_pmatView->getMat());
			mvp = XMMatrixMultiply(mvp, m_pmatProj->getMat());
			XMStoreFloat4x4(
				&m_cbMatrixData.mvp,
				DirectX::XMMatrixTranspose(mvp)
				);
			m_mvpChanged = false;
			needMatrices = true;
		}
	}

	// prepare the constant buffers to send them to the graphics device
	if (m_basicChanged)
	{
		m_d3dContext->UpdateSubresource(
			m_cbBasic.Get(),
			0,
			nullptr,
			&m_cbBasicData,
			0,
			0
			);
		m_basicChanged = false;

		m_d3dContext->VSSetConstantBuffers(
			0,
			1,
			m_cbBasic.GetAddressOf()
			);
		m_d3dContext->PSSetConstantBuffers(
			0,
			1,
			m_cbBasic.GetAddressOf()
			);
	}
	if (needMatrices)
	{
		m_d3dContext->UpdateSubresource(
			m_cbMatrix.Get(),
			0,
			nullptr,
			&m_cbMatrixData,
			0,
			0
			);

		m_d3dContext->VSSetConstantBuffers(
			1,
			1,
			m_cbMatrix.GetAddressOf()
			);
	}
	if (m_lightsChanged && (vertexShaderHints & SHADER_HINT_LIGHTS) || (pixelShaderHints & SHADER_HINT_LIGHTS))
	{
		m_d3dContext->UpdateSubresource(
			m_cbLights.Get(),
			0,
			nullptr,
			&m_cbLightsData,
			0,
			0
			);
		m_lightsChanged = false;

		if (vertexShaderHints & SHADER_HINT_LIGHTS)
		{
			m_d3dContext->VSSetConstantBuffers(
				2,
				1,
				m_cbLights.GetAddressOf()
				);
		}
		if (pixelShaderHints & SHADER_HINT_LIGHTS)
		{
			m_d3dContext->PSSetConstantBuffers(
				2,
				1,
				m_cbLights.GetAddressOf()
				);
		}
	}
}
