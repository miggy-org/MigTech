#include "pch.h"
#include "../core/MigUtil.h"
#include "OglObject.h"
#include "OglRender.h"
#include "AndroidApp.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OglObject::OglObject() :
	_verts(nullptr),
	_colors(nullptr),
	_norms(nullptr),
	_tex1(nullptr),
	_tex2(nullptr),
	_indices(nullptr),
	_type(GL_TRIANGLES),
	_numPts(0),
	_numInd(0),
	_offInd(0),
	_offIndCount(0),
	_inRenderSet(false)
{
	memset(_mappings, 0, sizeof(_mappings));
}

OglObject::~OglObject()
{
	if (_verts)
		delete _verts;
	if (_colors)
		delete _colors;
	if (_norms)
		delete _norms;
	if (_tex1)
		delete _tex1;
	if (_tex2)
		delete _tex2;
	if (_indices)
		delete _indices;
}

int OglObject::addShaderSet(const std::string& vs, const std::string& ps)
{
	OglRender* rendObj = (OglRender*)MigTech::MigUtil::theRend;
	OglProgram* program = rendObj->loadProgram(vs, ps);
	if (program != nullptr)
		_programs.push_back(program);
	return (program != nullptr ? _programs.size() - 1 : -1);
}

void OglObject::setImage(int index, const std::string& name, TXT_FILTER minFilter, TXT_FILTER magFilter, TXT_WRAP wrap)
{
	if (index < 0 || index > MAX_TEXTURE_MAPS)
		throw std::invalid_argument("(OglObject::setImage) Invalid index");
	if (minFilter == TXT_FILTER_NONE || magFilter == TXT_FILTER_NONE)
		throw std::invalid_argument("(OglObject::setImage) Invalid filter");
	if (wrap == TXT_WRAP_NONE)
		throw std::invalid_argument("(OglObject::setImage) Invalid wrap");

	OglImage* pimg = (OglImage*) MigUtil::theRend->getImage(name);
	if (pimg == nullptr)
		throw std::invalid_argument("(OglObject::setImage) Invalid image");

	_mappings[index].pimg = pimg;
	_mappings[index].minFilter = minFilter;
	_mappings[index].magFilter = magFilter;
	_mappings[index].wrap = wrap;
}

void OglObject::loadVertexBuffer(const void* pdata, unsigned int count, VDTYPE vdType)
{
	if (pdata == nullptr || count == 0)
		throw std::invalid_argument("(OglObject::loadVertexBuffer) Invalid vertex data");
	if (vdType == VDTYPE_UNKNOWN)
		throw std::invalid_argument("(OglObject::loadVertexBuffer) Invalid vertex data type");

	// free existing buffers
	if (_verts)
		delete _verts;
	_verts = nullptr;
	if (_colors)
		delete _colors;
	_colors = nullptr;
	if (_norms)
		delete _norms;
	_norms = nullptr;
	if (_tex1)
		delete _tex1;
	_tex1 = nullptr;
	if (_tex2)
		delete _tex2;
	_tex2 = nullptr;

	switch (vdType)
	{
	case VDTYPE_POSITION:
		{
			_verts = new GLfloat[3*count];

			const VertexPosition* pd = (VertexPosition*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
			}
		}
		break;
	case VDTYPE_POSITION_COLOR:
		{
			_verts = new GLfloat[3*count];
			_colors = new GLfloat[4*count];

			const VertexPositionColor* pd = (VertexPositionColor*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_colors[4*i+0] = pd[i].color.r;
				_colors[4*i+1] = pd[i].color.g;
				_colors[4*i+2] = pd[i].color.b;
				_colors[4*i+3] = pd[i].color.a;
			}
		}
		break;
	case VDTYPE_POSITION_COLOR_TEXTURE:
		{
			_verts = new GLfloat[3*count];
			_colors = new GLfloat[4*count];
			_tex1 = new GLfloat[2*count];

			const VertexPositionColorTexture* pd = (VertexPositionColorTexture*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_colors[4*i+0] = pd[i].color.r;
				_colors[4*i+1] = pd[i].color.g;
				_colors[4*i+2] = pd[i].color.b;
				_colors[4*i+3] = pd[i].color.a;
				_tex1[2*i+0] = pd[i].uv.x;
				_tex1[2*i+1] = pd[i].uv.y;
			}
		}
		break;
	case VDTYPE_POSITION_NORMAL:
		{
			_verts = new GLfloat[3*count];
			_norms = new GLfloat[3*count];

			const VertexPositionNormal* pd = (VertexPositionNormal*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_norms[3*i+0] = pd[i].norm.x;
				_norms[3*i+1] = pd[i].norm.y;
				_norms[3*i+2] = pd[i].norm.z;
			}
		}
		break;
	case VDTYPE_POSITION_NORMAL_TEXTURE:
		{
			_verts = new GLfloat[3*count];
			_norms = new GLfloat[3*count];
			_tex1 = new GLfloat[2*count];

			const VertexPositionNormalTexture* pd = (VertexPositionNormalTexture*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_norms[3*i+0] = pd[i].norm.x;
				_norms[3*i+1] = pd[i].norm.y;
				_norms[3*i+2] = pd[i].norm.z;
				_tex1[2*i+0] = pd[i].uv.x;
				_tex1[2*i+1] = pd[i].uv.y;
			}
		}
		break;
	case VDTYPE_POSITION_TEXTURE:
		{
			_verts = new GLfloat[3*count];
			_tex1 = new GLfloat[2*count];

			const VertexPositionTexture* pd = (VertexPositionTexture*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_tex1[2*i+0] = pd[i].uv.x;
				_tex1[2*i+1] = pd[i].uv.y;
			}
		}
		break;
	case VDTYPE_POSITION_TEXTURE_TEXTURE:
		{
			_verts = new GLfloat[3*count];
			_tex1 = new GLfloat[2*count];
			_tex2 = new GLfloat[2*count];

			const VertexPositionTextureTexture* pd = (VertexPositionTextureTexture*) pdata;
			for (int i = 0; i < count; i++)
			{
				_verts[3*i+0] = pd[i].pos.x;
				_verts[3*i+1] = pd[i].pos.y;
				_verts[3*i+2] = pd[i].pos.z;
				_tex1[2*i+0] = pd[i].uv1.x;
				_tex1[2*i+1] = pd[i].uv1.y;
				_tex2[2*i+0] = pd[i].uv2.x;
				_tex2[2*i+1] = pd[i].uv2.y;
			}
		}
		break;
	default:
		break;
	}
	_numPts = count;
}

void OglObject::loadIndexBuffer(const unsigned short* indices, unsigned int count, PRIMITIVE_TYPE type)
{
	if (indices == nullptr || count == 0)
		throw std::invalid_argument("(OglObject::loadIndexBuffer) Invalid vertex data");
	if (type == PRIMITIVE_TYPE_UNKNOWN)
		throw std::invalid_argument("(OglObject::loadIndexBuffer) Invalid primitive type");

	_indices = new GLshort[count];
	for (unsigned int i = 0; i < count; i++)
		_indices[i] = indices[i];
	_type = GL_TRIANGLES;
	if (type == PRIMITIVE_TYPE_TRIANGLE_STRIP)
		_type = GL_TRIANGLE_STRIP;
	else if (type == PRIMITIVE_TYPE_TRIANGLE_FAN)
		_type = GL_TRIANGLE_FAN;
	_numInd = _offIndCount = count;
}

static GLint toGLFilter(TXT_FILTER filt)
{
	switch (filt)
	{
	case TXT_FILTER_LINEAR: return GL_LINEAR;
	case TXT_FILTER_NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
	case TXT_FILTER_LINEAR_MIPMAP_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
	case TXT_FILTER_NEAREST_MIPMAP_LINEAR: return GL_NEAREST_MIPMAP_LINEAR;
	case TXT_FILTER_LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
	default: break;
	}
	return GL_NEAREST;
}

static GLint toGLWrap(TXT_WRAP wrap)
{
	switch (wrap)
	{
	case TXT_WRAP_CLAMP: return GL_CLAMP_TO_EDGE;
	case TXT_WRAP_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
	default: break;
	}
	return GL_REPEAT;
}

void OglObject::setIndexOffset(unsigned int offset, unsigned int count)
{
	_offInd = offset;
	_offIndCount = count;
}

int OglObject::getIndexOffset() const
{
	return _offInd;
}

int OglObject::getIndexCount() const
{
	return _offIndCount;
}

void OglObject::prepareRender(int shaderSet)
{
	OglRender* rendObj = (OglRender*)MigTech::MigUtil::theRend;

	// check to be sure the requested shader set has been loaded
	if (shaderSet >= _programs.size())
		throw std::invalid_argument("(OglObject::prepareRender) Invalid shader set");
	OglProgram* program = _programs[shaderSet];

	// activate the program
	program->useProgram();

	// load vertex data
	program->loadVerts(_verts);

	// load color data
	if (_colors)
		program->loadColors(_colors);

	// load normal data
	if (_norms)
		program->loadNorms(_norms);

	// load texture1 data
	if (_tex1)
		program->loadTex1Coords(_tex1);
	if (_mappings[0].pimg != nullptr && program->setTex1Location())
	{
		glActiveTexture(GL_TEXTURE0);
		_mappings[0].pimg->bindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(_mappings[0].minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(_mappings[0].magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(_mappings[0].wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(_mappings[0].wrap));
	}

	// load texture2 data
	if (_tex2)
		program->loadTex2Coords(_tex2);
	if (_mappings[1].pimg != nullptr && program->setTex2Location())
	{
		glActiveTexture(GL_TEXTURE1);
		_mappings[1].pimg->bindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(_mappings[1].minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(_mappings[1].magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(_mappings[1].wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(_mappings[1].wrap));
	}

	rendObj->setFaceCulling(_cull);
}

void OglObject::render(int shaderSet)
{
	// if we're not in a render sequence then prepare the render
	if (!_inRenderSet)
		prepareRender(shaderSet);

	OglProgram* program = _programs[shaderSet];

	// load basic object configuratino
	program->loadBasicConfig();

	// load matrices
	program->loadMatrices();

	// load lights
	program->loadLights();

	if (_indices)
		glDrawElements(_type, _offIndCount, GL_UNSIGNED_SHORT, &_indices[_offInd]);
	else
	    glDrawArrays(GL_TRIANGLES, 0, _numPts);
    //checkGLError("OglObject::render", "glDrawArrays");
}

void OglObject::startRenderSet(int shaderSet)
{
	prepareRender(shaderSet);

	_inRenderSet = true;
}

void OglObject::stopRenderSet()
{
	_inRenderSet = false;
}
