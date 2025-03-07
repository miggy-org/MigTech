#include "pch.h"
#include "OglShader.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OglShader::OglShader(GLuint id, GLenum type, unsigned int hints) :
	Shader(hints)
{
	_idShader = id;
	_typeShader = type;
}

OglShader::~OglShader()
{
}

Shader::Type OglShader::getType()
{
	if (_typeShader == GL_VERTEX_SHADER)
		return SHADER_TYPE_VERTEX;
	else if (_typeShader == GL_FRAGMENT_SHADER)
		return SHADER_TYPE_PIXEL;

	return SHADER_TYPE_NONE;
}
