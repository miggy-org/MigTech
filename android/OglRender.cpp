#include "pch.h"
#include "../core/MigUtil.h"
#include "OglRender.h"
#include "OglMatrix.h"
#include "OglShader.h"
#include "OglObject.h"
#include "AndroidApp.h"

extern "C" {
#include "../core/libjpeg/jpeglib.h"
}

extern "C" {
#include "../core/libpng/png.h"
}

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OglRender::OglRender() :
	_outputSize(), _clearColor(0, 0, 0)
{
}

OglRender::~OglRender()
{
}

void OglRender::createDeviceIndependentResources()
{
}

void OglRender::createDeviceResources()
{
}

void OglRender::createWindowSizeDependentResources()
{
}

static void printGLString(const char *name, GLenum s)
{
    const char *v = (const char *) glGetString(s);
	LOGINFO("(::printGLString) GL %s = %s", name, v);
}

bool OglRender::initRenderer()
{
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    //printGLString("Extensions", GL_EXTENSIONS);	// warning - very long string

	createDeviceIndependentResources();
	createDeviceResources();

	return true;
}

void OglRender::termRenderer()
{
	{
		std::map<std::string, OglShader*>::const_iterator iter;
		for (iter = _shaders.begin(); iter != _shaders.end(); iter++)
		{
			delete iter->second;
		}
	}
	_shaders.clear();

	{
		std::map<std::string, OglProgram*>::const_iterator iter;
		for (iter = _programs.begin(); iter != _programs.end(); iter++)
		{
			delete iter->second;
		}
	}
	_programs.clear();

	{
		std::map<std::string, OglImage*>::const_iterator iter;
		for (iter = _images.begin(); iter != _images.end(); iter++)
		{
			delete iter->second;
		}
	}
	_images.clear();
}

IMatrix* OglRender::createMatrix()
{
	return (IMatrix*) new OglMatrix();
}

void OglRender::deleteMatrix(IMatrix* pmat)
{
	delete (OglMatrix*)pmat;
}

void OglRender::setProjectionMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
	{
		OglMatrix* pomat = (OglMatrix*)pmat;
		_perspective.copy(pomat);
	}
	else
		_perspective.identity();
}

void OglRender::setProjectionMatrix(float angleY, float aspect, float nearZ, float farZ, bool useOrientation)
{
	_perspective.loadPerspectiveFovRH(angleY, aspect, nearZ, farZ);
}

void OglRender::setViewMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
	{
		OglMatrix* pomat = (OglMatrix*)pmat;
		_view.copy(pomat);
	}
	else
		_view.identity();
}

void OglRender::setViewMatrix(Vector3 eyePos, Vector3 focusPos, Vector3 upVector)
{
	_view.loadLookAtRH(eyePos, focusPos, upVector);
}

void OglRender::setModelMatrix(const IMatrix* pmat)
{
	if (pmat != nullptr)
	{
		OglMatrix* pomat = (OglMatrix*)pmat;
		_model.copy(pomat);
	}
	else
		_model.identity();
}

void OglRender::setOutputSize(Size newSize)
{
	_outputSize = newSize;

    glViewport(0, 0, newSize.width, newSize.height);
    //checkGlError("glViewport");
}

Size OglRender::getOutputSize()
{
	return _outputSize;
}

void OglRender::setViewport(const Rect* newPort, bool clearRenderBuffer, bool clearDepthBuffer)
{
	// reset the viewport
	if (newPort != nullptr)
	{
		Rect vp = Rect(
			newPort->corner.x * _outputSize.width,
			newPort->corner.y * _outputSize.height,
			newPort->size.width * _outputSize.width,
			newPort->size.height * _outputSize.height
			);
		glViewport(vp.corner.x, vp.corner.y, vp.size.width, vp.size.height);
	}
	else
		glViewport(0, 0, _outputSize.width, _outputSize.height);

	// set the clear color
	GLbitfield clearMask = 0;
	if (clearRenderBuffer)
		clearMask |= GL_COLOR_BUFFER_BIT;
	if (clearDepthBuffer)
		clearMask |= GL_DEPTH_BUFFER_BIT;
	if (clearMask)
		glClear(clearMask);
}

static GLuint loadShader(GLenum shaderType, const char* pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
	{
		glShaderSource(shader, 1, &pSource, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
		{
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
			{
                char* buf = (char*) malloc(infoLen);
                if (buf)
				{
					glGetShaderInfoLog(shader, infoLen, nullptr, buf);
					LOGERR("(::loadShader) Could not compile shader %d:\n%s", shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

Shader* OglRender::loadVertexShader(const std::string& name, VDTYPE vdType, unsigned int shaderHints)
{
	if (vdType == VDTYPE_UNKNOWN)
		throw std::invalid_argument("(OglRender::loadVertexShader) No input layout specified");

	// see if the shader already exists
	Shader* ps = getShader(name);
	if (ps != nullptr)
	{
		// must match requested shader type
		if (ps->getType() != Shader::SHADER_TYPE_VERTEX)
			throw std::runtime_error("(OglRender::loadVertexShader) Existing shader doesn't match requested shader type");
		return ps;
	}

	char gVertexShader[4096];
	if (AndroidUtil_getAssetBuffer(name + ".vert", gVertexShader, sizeof(gVertexShader)) != -1)
	{
		LOGINFO("(OglRender::loadVertexShader) Compiling shader %s", name.c_str());

		GLuint shader = loadShader(GL_VERTEX_SHADER, gVertexShader);
		if (shader)
		{
			ps = new OglShader(shader, GL_VERTEX_SHADER, shaderHints);
			_shaders[name] = (OglShader*) ps;
		}
		else
			throw std::runtime_error("(OglRender::loadVertexShader) Could not compile shader");
	}

    return ps;
}

Shader* OglRender::loadPixelShader(const std::string& name, unsigned int shaderHints)
{
	// see if the shader already exists
	Shader* ps = getShader(name);
	if (ps != nullptr)
	{
		// must match requested shader type
		if (ps->getType() != Shader::SHADER_TYPE_PIXEL)
			throw std::runtime_error("(OglRender::loadPixelShader) Existing shader doesn't match requested shader type");
		return ps;
	}

	char gFragmentShader[4096];
	if (AndroidUtil_getAssetBuffer(name + ".frag", gFragmentShader, sizeof(gFragmentShader)) != -1)
	{
		LOGINFO("(OglRender::loadPixelShader) Compiling shader %s", name.c_str());

		GLuint shader = loadShader(GL_FRAGMENT_SHADER, gFragmentShader);
		if (shader)
		{
			ps = new OglShader(shader, GL_FRAGMENT_SHADER, shaderHints);
			_shaders[name] = (OglShader*) ps;
		}
		else
			throw std::runtime_error("(OglRender::loadPixelShader) Could not compile shader");
	}
    return ps;
}

Shader* OglRender::getShader(const std::string& name)
{
	std::map<std::string, OglShader*>::const_iterator iter = _shaders.find(name);
	if (iter != _shaders.end() && iter->second != nullptr)
		return iter->second;
	return nullptr;
}

static OglImage* loadJPEGImage(const std::string& name, unsigned int loadFlags)
{
	// get the asset file size
	int len = AndroidUtil_getAssetBuffer(name, nullptr, 0);
	if (len == -1)
	{
		LOGWARN("(OglRender::loadJPEGImage) image '%s' doesn't exist", name.c_str());
		return nullptr;
	}
	//MigUtil::debug("(OglRender::loadJPEGImage) image '%s' file size is %d", name.c_str(), len);

	// load the image into a memory buffer
	byte* pFile = new byte[len];
	if (AndroidUtil_getAssetBuffer(name, pFile, len) == -1)
	{
		LOGWARN("(OglRender::loadJPEGImage) image '%s' failed to load", name.c_str());
		delete [] pFile;
		return nullptr;
	}
	byte* pData = nullptr;

	// initialize decompression
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, pFile, len);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	LOGINFO("(OglRender::loadJPEGImage) Image=%s, w=%d, h=%d", name.c_str(), cinfo.output_width, cinfo.output_height);

	// for now, only 24-bit JPEGs
	if (cinfo.output_components == 3)
	{
		if (loadFlags == LOAD_IMAGE_NONE)
		{
			// read into the image buffer quickly
			int row_stride = cinfo.output_width*cinfo.output_components;
			pData = new byte[row_stride*cinfo.output_height];
			if (pData != nullptr)
			{
				JSAMPROW row_pointer[1];
				while (cinfo.output_scanline < cinfo.output_height)
				{
					row_pointer[0] = pData + cinfo.output_scanline*row_stride;
					jpeg_read_scanlines(&cinfo, row_pointer, 1);
				}
			}
		}
		else
		{
			// read into the image buffer
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
				delete [] psrc;

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
	}
	else
	{
		LOGWARN("(OglRender::loadJPEGImage) %d color components not supported", cinfo.output_components);
	}

	// clean up decompression
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete [] pFile;

	// load the data into an Image and return
	OglImage* newImage = nullptr;
	if (pData != nullptr)
	{
		IMG_FORMAT fmt = IMG_FORMAT_RGB;
		if (loadFlags != LOAD_IMAGE_NONE)
		{
			if (loadFlags & LOAD_IMAGE_DROP_COLOR)
				fmt = IMG_FORMAT_ALPHA;
			else
				fmt = IMG_FORMAT_RGBA;
		}

		// create the image object and load
		newImage = new OglImage();
		newImage->loadTexture(fmt, cinfo.output_width, cinfo.output_height, pData);
		delete pData;
	}
	return newImage;
}

struct USER_READ_DATA
{
	byte* pFile;
	int sizeFile;
	int currOffset;
};

static void userReadData(png_structp read_ptr, png_bytep data, png_size_t length)
{
	if (read_ptr != nullptr)
	{
		USER_READ_DATA* pData = (USER_READ_DATA*) png_get_io_ptr(read_ptr);
		if (pData != nullptr)
		{
			if (pData->currOffset + length <= pData->sizeFile)
			{
				//MigUtil::debug("(OglRender::userReadData) Reading %d bytes from offset %d", length, pData->currOffset);
				memcpy(data, &(pData->pFile[pData->currOffset]), length);
				pData->currOffset += length;
			}
			else
				LOGERR("(OglRender::userReadData) PNG read error, EOF");
		}
		else
			LOGERR("(OglRender::userReadData) PNG read error, pData was nullptr");
	}
	else
		LOGERR("(OglRender::userReadData) PNG read error, read_ptr was nullptr");
}

static OglImage* loadPNGImage(const std::string& name, unsigned int loadFlags)
{
	// get the asset file size
	int len = AndroidUtil_getAssetBuffer(name, nullptr, 0);
	if (len == -1)
	{
		LOGWARN("(OglRender::loadPNGImage) image '%s' doesn't exist", name.c_str());
		return nullptr;
	}
	//MigUtil::debug("(OglRender::loadPNGImage) image '%s' file size is %d", name.c_str(), len);

	// load the image into a memory buffer
	byte* pFile = new byte[len];
	if (AndroidUtil_getAssetBuffer(name, pFile, len) == -1)
	{
		LOGWARN("(OglRender::loadPNGImage) image '%s' failed to load", name.c_str());
		delete [] pFile;
		return nullptr;
	}

	// check the header to ensure it's a PNG
	if (png_sig_cmp(pFile, 0, 8))
	{
		LOGWARN("(OglRender::loadPNGImage) image '%s' does not appear to be a PNG", name.c_str());
		delete [] pFile;
		return nullptr;
	}

	// allocate needed structs
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
	{
		LOGWARN("(OglRender::loadPNGImage) png_create_read_struct() failed");
		delete [] pFile;
		return nullptr;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		LOGWARN("(OglRender::loadPNGImage) png_create_info_struct() failed");
		png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
		delete [] pFile;
		return nullptr;
	}

	// init the PNG file IO
	USER_READ_DATA userReadDataStruct;
	userReadDataStruct.pFile = pFile;
	userReadDataStruct.sizeFile = len;
	userReadDataStruct.currOffset = 0;
	png_set_read_fn(png_ptr, &userReadDataStruct, userReadData);

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
	LOGINFO("(OglRender::loadPNGImage) Image=%s, w=%d, h=%d, bits=%d, channels=%d", name.c_str(), imageWidth, imageHeight, bitDepth, channels);
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

			// drop color components if so flagged
			if (loadFlags & LOAD_IMAGE_DROP_COLOR)
			{
				int numPixels = imageWidth * imageHeight;
				byte* pNewData = new byte[numPixels];
				for (int i = 0; i < numPixels; i++)
				{
					pNewData[i] = pData[4 * i + 3];
				}

				delete [] pData;
				pData = pNewData;
			}
		}
	}
	else
	{
		LOGWARN("(OglRender::loadPNGImage) Unsupported bit depth or channels (%d, %d)", bitDepth, channels);
	}
	
	// clean up
	png_destroy_read_struct(&png_ptr, (png_infopp)&info_ptr, (png_infopp)nullptr);
	delete [] pFile;

	// load the data into an Image and return
	OglImage* newImage = nullptr;
	if (pData != nullptr)
	{
		// create the image object and load
		newImage = new OglImage();
		newImage->loadTexture(loadFlags & LOAD_IMAGE_DROP_COLOR ? IMG_FORMAT_ALPHA : IMG_FORMAT_RGBA, imageWidth, imageHeight, pData);
		delete pData;
	}
	return newImage;
}

Image* OglRender::loadImage(const std::string& name, const std::string& path, unsigned int loadFlags)
{
	// see if the image already exists
	Image* pi = getImage(name);
	if (pi != nullptr)
		return pi;

	OglImage* newImage = nullptr;
	int findDot = path.rfind(".");
	if (findDot != string::npos)
	{
		std::string ext = path.substr(findDot + 1);
		if (0 == ext.compare("jpg") ||
			0 == ext.compare("jpeg"))
		{
			newImage = loadJPEGImage(path, loadFlags);
		}
		else if (0 == ext.compare("png"))
		{
			newImage = loadPNGImage(path, loadFlags);
		}
	}
	if (newImage != nullptr)
		_images[name] = newImage;

	return newImage;
}

Image* OglRender::getImage(const std::string& name)
{
	std::map<std::string, OglImage*>::const_iterator iter = _images.find(name);
	if (iter != _images.end() && iter->second != nullptr)
		return iter->second;
	return nullptr;
}

Image* OglRender::createRenderTarget(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint)
{
	// if the render target already exists, that is considered an error
	if (getImage(name) != nullptr)
		return nullptr;

	OglRenderTarget* newTarget = new OglRenderTarget();
	if (!newTarget->init(fmtHint, width, height, depthBitsHint))
	{
		delete newTarget;
		return nullptr;
	}

	_images[name] = newTarget;
	return newTarget;
}

void OglRender::unloadImage(const std::string& name)
{
	std::map<std::string, OglImage*>::iterator iter = _images.find(name);
	if (iter != _images.end() && iter->second != nullptr)
	{
		delete iter->second;
		_images.erase(iter);
	}
}

Object* OglRender::createObject()
{
	return new OglObject();
}

void OglRender::deleteObject(Object* pobj)
{
	delete (OglObject*)pobj;
}

void OglRender::setClearColor(const Color& clearCol)
{
	_clearColor = clearCol;
}

void OglRender::setObjectColor(const Color& objCol)
{
	_objColor = objCol;
}

void OglRender::setBlending(BLEND_STATE blend)
{
	if (blend != BLEND_STATE_NONE)
	{
		glEnable(GL_BLEND);
		switch (blend)
		{
		case BLEND_STATE_SRC_ALPHA:				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
		case BLEND_STATE_ONE_MINUS_SRC_ALPHA:	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA); break;
		case BLEND_STATE_DST_ALPHA:				glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA); break;
		case BLEND_STATE_ONE_MINUS_DST_ALPHA:	glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA); break;
		default: break;
		}
	}
	else
		glDisable(GL_BLEND);
}

void OglRender::setDepthTesting(DEPTH_TEST_STATE depth, bool enableWrite)
{
	if (depth != DEPTH_TEST_STATE_NONE)
	{
		glEnable(GL_DEPTH_TEST);
		switch (depth)
		{
		case DEPTH_TEST_STATE_LESS:		glDepthFunc(GL_LESS); break;
		case DEPTH_TEST_STATE_LEQUAL:	glDepthFunc(GL_LEQUAL); break;
		case DEPTH_TEST_STATE_EQUAL:	glDepthFunc(GL_EQUAL); break;
		case DEPTH_TEST_STATE_GEQUAL:	glDepthFunc(GL_GEQUAL); break;
		case DEPTH_TEST_STATE_GREATER:	glDepthFunc(GL_GREATER); break;
		case DEPTH_TEST_STATE_NEQUAL:	glDepthFunc(GL_NOTEQUAL); break;
		case DEPTH_TEST_STATE_ALWAYS:	glDepthFunc(GL_ALWAYS); break;
		default: break;
		}
		glDepthMask(enableWrite);
	}
	else
		glDisable(GL_DEPTH_TEST);
}

void OglRender::setFaceCulling(FACE_CULLING cull)
{
	if (cull != FACE_CULLING_NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(cull == FACE_CULLING_FRONT ? GL_FRONT : GL_BACK);
	}
	else
		glDisable(GL_CULL_FACE);
}

void OglRender::setMiscValue(int index, float value)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::setMiscValue) Misc index out of bounds");
	_misc[index] = value;
}

void OglRender::setAmbientColor(const Color& ambientCol)
{
	_ambColor = ambientCol;
}

void OglRender::setLightColor(int index, const Color& litCol)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::setLightColor) Light index out of bounds");
	_litColor[index] = litCol;
}

void OglRender::setLightDirPos(int index, const Vector3& litDirPos, bool isDir)
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::setLightDirPos) Light index out of bounds");
	_litDirPos[index] = litDirPos;
	_litIsDir[index] = isDir;
}

void OglRender::onSuspending()
{
}

void OglRender::onResuming()
{
}

static void toGLViewport(const Rect& newPort, float width, float height)
{
	glViewport(
		newPort.corner.x * width,
		newPort.corner.y * height,
		newPort.size.width * width,
		newPort.size.height * height);
}

void OglRender::preRender(int pass, RenderPass* passObj)
{
	unsigned int configBits = 0;
	if (pass > 0 && passObj != nullptr)
		configBits = passObj->getConfigBits();

	// get render target
	OglRenderTarget* target = nullptr;
	if (configBits & RenderPass::USE_RENDER_TARGET)
		target = (OglRenderTarget*)passObj->getRenderTarget();

	if (target != nullptr)
	{
		// bind the frame buffer object as the render target
		target->bindRenderTarget();

		// match the viewport to the render target
		glViewport(0, 0, target->getWidth(), target->getHeight());
	}
	else
	{
		// reset the rendering target
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (configBits & RenderPass::USE_VIEW_PORT)
		{
			// match the viewport to the render target
			const Rect& viewPort = passObj->getViewPort();
			glViewport(
				viewPort.corner.x * _outputSize.width,
				viewPort.corner.y * _outputSize.height,
				viewPort.size.width * _outputSize.width,
				viewPort.size.height * _outputSize.height);
		}
		else
		{
			// reset the viewport
			glViewport(0, 0, _outputSize.width, _outputSize.height);
		}
	}

	// set the clear color
	if (configBits & RenderPass::USE_CLEAR_COLOR)
	{
		const Color& clearColor = passObj->getClearColor();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	}
	else
		glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);

	// clear the render target buffers
	GLbitfield clearMask = 0;
	if (pass == 0 || (configBits & RenderPass::USE_CLEAR_COLOR))
		clearMask |= GL_COLOR_BUFFER_BIT;
	if (pass == 0 || (configBits & RenderPass::USE_CLEAR_DEPTH))
		clearMask |= GL_DEPTH_BUFFER_BIT;
	if (clearMask)
		glClear(clearMask);

	// first config int will contain the pass (1 based, 0 means final pass, -1 means overlay pass)
	_cfg[0] = pass;
}

void OglRender::postRender(int pass, RenderPass* passObj)
{
}

void OglRender::present()
{
}

OglProgram* OglRender::loadProgram(const std::string& vs, const std::string& ps)
{
	// produce the look-up key
	std::string key = vs + ":" + ps;

	// see if the program already exists, and return it if it does
	std::map<std::string, OglProgram*>::const_iterator iter = _programs.find(key);
	if (iter != _programs.end() && iter->second != nullptr)
		return iter->second;

	// build a new program
	OglProgram* newProgram = new OglProgram();
	newProgram->buildProgram(vs, ps);
	_programs[key] = newProgram;
	return newProgram;
}

void OglRender::loadMVPMatrix(GLint location) const
{
	OglMatrix mvp = _model;
	mvp.multiply(&_view);
	mvp.multiply(&_perspective);
	glUniformMatrix4fv(location, 1, GL_FALSE, mvp.getData());
}

void OglRender::loadModelMatrix(GLint location) const
{
	glUniformMatrix4fv(location, 1, GL_FALSE, _model.getData());
}

void OglRender::loadViewMatrix(GLint location) const
{
	glUniformMatrix4fv(location, 1, GL_FALSE, _view.getData());
}

void OglRender::loadProjMatrix(GLint location) const
{
	glUniformMatrix4fv(location, 1, GL_FALSE, _perspective.getData());
}

const Color& OglRender::getObjectColor() const
{
	return _objColor;
}

const float* OglRender::getMiscVal() const
{
	return _misc;
}

const int* OglRender::getConfigVal() const
{
	return _cfg;
}

const Color& OglRender::getAmbientColor() const
{
	return _ambColor;
}

const Color& OglRender::getLightColor(int index) const
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::getLightColor) Light index out of bounds");
	return _litColor[index];
}

const Vector3& OglRender::getLightDirPos(int index) const
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::getLightDirPos) Light index out of bounds");
	return _litDirPos[index];
}

bool OglRender::getLightIsDir(int index) const
{
	if (index < 0 || index > 3)
		throw std::out_of_range("(OglRender::getLightIsDir) Light index out of bounds");
	return _litIsDir[index];
}
