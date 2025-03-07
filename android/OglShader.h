#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Shader.h"

namespace MigTech
{
	// OpenGL version of the MigTech shader
	class OglShader : public Shader
	{
	protected:
		GLuint _idShader;
		GLenum _typeShader;

	public:
		GLuint getShaderID() { return _idShader; }

	public:
		OglShader(GLuint id, GLenum type, unsigned int hints);
		virtual ~OglShader();

		virtual Type getType();
	};
}