#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include <map>

#include "../core/MigDefines.h"
#include "../core/RenderBase.h"
#include "OglShader.h"
#include "OglProgram.h"
#include "OglImage.h"
#include "OglMatrix.h"

namespace MigTech
{
	// OpenGL version of the MigTech renderer
	class OglRender : public RenderBase
	{
	protected:
		virtual void createDeviceIndependentResources();
		virtual void createDeviceResources();
		virtual void createWindowSizeDependentResources();

	public:
		OglRender();
		virtual ~OglRender();

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
		// OpenGL specific
		OglProgram* loadProgram(const std::string& vs, const std::string& ps);
		void loadMVPMatrix(GLint location) const;
		void loadModelMatrix(GLint location) const;
		void loadViewMatrix(GLint location) const;
		void loadProjMatrix(GLint location) const;
		const Color& getObjectColor() const;
		const float* getMiscVal() const;
		const int* getConfigVal() const;
		const Color& getAmbientColor() const;
		const Color& getLightColor(int index) const;
		const Vector3& getLightDirPos(int index) const;
		bool getLightIsDir(int index) const;

	protected:
		// Cached device properties.
		Size	_outputSize;
		Color   _clearColor;
		Color   _objColor;
		float	_misc[4];
		int		_cfg[4];
		Color   _ambColor;
		Color   _litColor[4];
		Vector3 _litDirPos[4];
		bool    _litIsDir[4];

		// Supported matrices
		OglMatrix _perspective;
		OglMatrix _view;
		OglMatrix _model;

		// Shader list
		std::map<std::string, OglShader*> _shaders;

		// Program list
		std::map<std::string, OglProgram*> _programs;

		// Image map list
		std::map<std::string, OglImage*> _images;
	};
}