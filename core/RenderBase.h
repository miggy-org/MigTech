#pragma once

#include "MigDefines.h"
#include "MigBase.h"
#include "Matrix.h"
#include "Image.h"
#include "Shader.h"
#include "Object.h"

namespace MigTech
{
	class RenderPass : public MigBase
	{
	public:
		// render pass type
		enum RenderPassType { RENDER_PASS_NONE, RENDER_PASS_PRE, RENDER_PASS_POST };

		// config bits
		static const unsigned int USE_CLEAR_COLOR   = 1;
		static const unsigned int USE_CLEAR_DEPTH   = 2;
		static const unsigned int USE_VIEW_PORT     = 4;
		static const unsigned int USE_RENDER_TARGET = 8;

	public:
		RenderPass(RenderPassType type);
		virtual ~RenderPass();

		// init routines
		bool init(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint);
		bool init(const Rect& viewPort);
		bool init();

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual bool preRender();
		virtual void postRender();

		// checks to see if the pass is configured properly
		virtual bool isValid() const;

		// returns render target info
		Image* getRenderTarget() { return ((_config & USE_RENDER_TARGET) ? _target : nullptr); }
		const std::string& getRenderTargetName() const { return _name; }

		const RenderPassType getType() const { return _type; }
		unsigned int getConfigBits() const { return _config; }

		const Matrix& getProjMatrix() const { return _proj;	}
		void setProjMatrix(const Matrix& newProj) { _proj = newProj; }
		const Matrix& getViewMatrix() const { return _view; }
		void setViewMatrix(const Matrix& newView) { _view = newView; }
		const Color& getClearColor() const { return _clearColor; }
		void setClearColor(const Color& newColor) { _clearColor = newColor; }
		const Rect& getViewPort() const { return _viewPort; }
		void setViewPort(const Rect& newPort) { _viewPort = newPort; }

	protected:
		// type
		RenderPassType _type;

		// config
		unsigned int _config;

		// render target
		Image* _target;
		std::string _name;
		IMG_FORMAT _fmtHint;
		int _width;
		int _height;
		int _depthBitsHint;

		Matrix _proj;
		Matrix _view;
		Color _clearColor;
		Rect _viewPort;
	};

	class RenderBase
	{
	protected:
		virtual void createDeviceIndependentResources() = 0;
		virtual void createDeviceResources() = 0;
		virtual void createWindowSizeDependentResources() = 0;

	public:
		RenderBase() { }
		virtual ~RenderBase() { }

		virtual bool initRenderer() = 0;
		virtual void termRenderer() = 0;

		virtual IMatrix* createMatrix() = 0;
		virtual void deleteMatrix(IMatrix* pmat) = 0;
		virtual void setProjectionMatrix(const IMatrix* pmat) = 0;
		virtual void setProjectionMatrix(float angleY, float aspect, float nearZ, float farZ, bool useOrientation) = 0;
		virtual void setViewMatrix(const IMatrix* pmat) = 0;
		virtual void setViewMatrix(Vector3 eyePos, Vector3 focusPos, Vector3 upVector) = 0;
		virtual void setModelMatrix(const IMatrix* pmat) = 0;

		virtual Shader* loadVertexShader(const std::string& name, VDTYPE vdType, unsigned int shaderHints) = 0;
		virtual Shader* loadPixelShader(const std::string& name, unsigned int shaderHints) = 0;
		virtual Shader* getShader(const std::string& name) = 0;

		virtual Image* loadImage(const std::string& name, const std::string& path, unsigned int loadFlags) = 0;
		virtual Image* getImage(const std::string& name) = 0;
		virtual Image* createRenderTarget(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint) = 0;
		virtual void unloadImage(const std::string& name) = 0;

		virtual Object* createObject() = 0;
		virtual void deleteObject(Object* pobj) = 0;

		virtual void setOutputSize(Size newSize) = 0;
		virtual Size getOutputSize() = 0;
		virtual void setViewport(const Rect* newPort, bool clearRenderBuffer, bool clearDepthBuffer) = 0;

		virtual void setClearColor(const Color& clearCol) = 0;
		virtual void setObjectColor(const Color& objCol) = 0;
		virtual void setBlending(BLEND_STATE blend) = 0;
		virtual void setDepthTesting(DEPTH_TEST_STATE depth, bool enableWrite) = 0;
		virtual void setFaceCulling(FACE_CULLING cull) = 0;

		virtual void setMiscValue(int index, float value) = 0;
		virtual void setAmbientColor(const Color& ambientCol) = 0;
		virtual void setLightColor(int index, const Color& litCol) = 0;
		virtual void setLightDirPos(int index, const Vector3& litDirPos, bool isDir) = 0;

		virtual void onSuspending() = 0;
		virtual void onResuming() = 0;

		virtual void preRender(int pass, RenderPass* passObj) = 0;
		virtual void postRender(int pass, RenderPass* passObj) = 0;
		virtual void present() = 0;
	};
}