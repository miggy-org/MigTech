#include "pch.h"
#include "../core/MigUtil.h"
#include "OglImage.h"
#include "AndroidApp.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OglImage::OglImage()
{
	glGenTextures(1, &_textureID);
}

OglImage::~OglImage()
{
	glDeleteTextures(1, &_textureID);
}

void OglImage::bindTexture()
{
	glBindTexture(GL_TEXTURE_2D, _textureID);
}

void OglImage::loadTexture(IMG_FORMAT fmt, int width, int height, void* pData)
{
	GLenum format;
	if (fmt == IMG_FORMAT_ALPHA)
		format = GL_ALPHA;
	else if (fmt == IMG_FORMAT_GREYSCALE)
		format = GL_LUMINANCE;
	else if (fmt == IMG_FORMAT_RGB)
		format = GL_RGB;
	else if (fmt == IMG_FORMAT_RGBA)
		format = GL_RGBA;
	else
		throw std::invalid_argument("(OglImage::loadTexture) No format provided");

	bindTexture();
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pData);
	checkGLError("OglImage::loadTexture", "glTexImage2D");

	_width = width;
	_height = height;
}

OglRenderTarget::OglRenderTarget() : OglImage()
{
	_caps = IMAGE_CAPS_RENDER_TARGET | IMAGE_CAPS_BOTTOM_UP;

	_frameBufferID = 0;
	_depthBufferID = 0;
}

OglRenderTarget::~OglRenderTarget()
{
	if (_frameBufferID > 0)
		glDeleteRenderbuffers(1, &_frameBufferID);
	if (_depthBufferID > 0)
		glDeleteRenderbuffers(1, &_depthBufferID);
}

bool OglRenderTarget::init(IMG_FORMAT fmtHint, int width, int height, int depthBitsHint)
{
	// generate the frame buffer object
	glGenFramebuffers(1, &_frameBufferID);

	// generate the color texture that will be the rendering target (note this was done in base class constructor)
	//glGenTextures(1, &_textureID);

	// generate the depth buffer
	if (depthBitsHint > 0)
		glGenRenderbuffers(1, &_depthBufferID);

	// bind the frame buffer object
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);

	// create the shadow texture map
	loadTexture(fmtHint, width, height, nullptr);
	//glBindTexture(GL_TEXTURE_2D, _textureID);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	//if (checkGLError("OglImage::loadTexture", "glTexImage2D") != GL_NO_ERROR)
	//	return false;

	// attach the color texture map to the frame buffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);
	if (checkGLError("OglImage::loadTexture", "glFramebufferTexture2D") != GL_NO_ERROR)
		return false;

	// bind the depth buffer object
	if (_depthBufferID > 0)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferID);

		// create the depth buffer
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		if (checkGLError("OglImage::loadTexture", "glRenderbufferStorage") != GL_NO_ERROR)
			return false;

		// attach the depth buffer map to the frame buffer object
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBufferID);
		if (checkGLError("OglImage::loadTexture", "glFramebufferRenderbuffer") != GL_NO_ERROR)
			return false;
	}

	// check the final FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGWARN("(OglImage::loadTexture) glCheckFramebufferStatus returned %d", status);
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_width = width;
	_height = height;
	return true;
}

void OglRenderTarget::bindRenderTarget()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);
	checkGLError("OglImage::bindRenderTarget", "glBindFramebuffer");
}
