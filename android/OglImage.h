#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Image.h"

namespace MigTech
{
	// OpenGL version of a MigTech image map
	class OglImage : public Image
	{
	protected:
		GLuint _textureID;

	public:
		OglImage();
		virtual ~OglImage();

		void bindTexture();
		void loadTexture(IMG_FORMAT fmt, int width, int height, void* pData);
	};

	// OpenGL version of a render target
	class OglRenderTarget : public OglImage
	{
	protected:
		GLuint _frameBufferID;
		GLuint _depthBufferID;

	public:
		OglRenderTarget();
		virtual ~OglRenderTarget();

		bool init(IMG_FORMAT fmtHint, int width, int height, int depthBitsHint);
		void bindRenderTarget();
	};
}
