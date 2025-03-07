#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Object.h"
#include "OglImage.h"
//#include "OglShader.h"
#include "OglProgram.h"

namespace MigTech
{
	struct TxtMapping
	{
		OglImage* pimg;
		// TODO: these should be the GL equivalents, so we aren't converting at runtime
		TXT_FILTER minFilter;
		TXT_FILTER magFilter;
		TXT_WRAP wrap;
	};

	// OpenGL version of a MigTech object
	class OglObject : public Object
	{
	protected:
		std::vector<OglProgram*> _programs;

		// model data arrays
		GLfloat* _verts;
		GLfloat* _colors;
		GLfloat* _norms;
		GLfloat* _tex1;
		GLfloat* _tex2;
		GLshort* _indices;
		GLenum _type;
		GLsizei _numPts;
		GLsizei _numInd;
		GLint _offInd;
		GLsizei _offIndCount;

		// texture mappings
		TxtMapping _mappings[MAX_TEXTURE_MAPS];

		bool _inRenderSet;

	protected:
		void prepareRender(int shaderSet);

	public:
		OglObject();
		virtual ~OglObject();

		virtual int addShaderSet(const std::string& vs, const std::string& ps);
		virtual void setImage(int index, const std::string& name, TXT_FILTER minFilter, TXT_FILTER magFilter, TXT_WRAP wrap);
		virtual void loadVertexBuffer(const void* pdata, unsigned int count, VDTYPE vdType);
		virtual void loadIndexBuffer(const unsigned short* indices, unsigned int count, PRIMITIVE_TYPE type);

		virtual void setIndexOffset(unsigned int offset, unsigned int count);
		virtual int getIndexOffset() const;
		virtual int getIndexCount() const;

		virtual void render(int shaderSet = 0);

		virtual void startRenderSet(int shaderSet = 0);
		virtual void stopRenderSet();
	};
}
