#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "OglShader.h"

namespace MigTech
{
	class OglProgram
	{
	protected:
		GLuint _program;

		// shader variable position handles
		GLuint _gvPositionHandle;
		GLuint _gvColorHandle;
		GLuint _gvNormHandle;
		GLuint _gvTex1Handle;
		GLuint _gvTex2Handle;
		GLint _modelLocation;
		GLint _viewLocation;
		GLint _projLocation;
		GLint _mvpLocation;
		GLint _objColorLocation;
		GLint _texture1Location;
		GLint _texture2Location;
		GLint _miscValLocation;
		GLint _cfgValLocation;
		GLint _ambientColorLocation;
		GLint _litColorLocation[4];
		GLint _litDirPosLocation[4];

	public:
		OglProgram();
		~OglProgram();

		void buildProgram(const std::string& vs, const std::string& ps);
		void useProgram();

		// geometry
		bool loadVerts(GLfloat* verts);
		bool loadColors(GLfloat* colors);
		bool loadNorms(GLfloat* norms);
		bool loadTex1Coords(GLfloat* tex1);
		bool setTex1Location();
		bool loadTex2Coords(GLfloat* tex2);
		bool setTex2Location();

		// non-geometry
		void loadBasicConfig();
		void loadMatrices();
		void loadLights();
	};
}
