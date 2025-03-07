#include "pch.h"
#include "../core/MigUtil.h"
#include "OglProgram.h"
#include "OglRender.h"
#include "AndroidApp.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OglProgram::OglProgram() :
	_program(0),
	_gvPositionHandle(-1),
	_gvColorHandle(-1),
	_gvNormHandle(-1),
	_gvTex1Handle(-1),
	_gvTex2Handle(-1),
	_modelLocation(-1),
	_viewLocation(-1),
	_projLocation(-1),
	_mvpLocation(-1),
	_objColorLocation(-1),
	_texture1Location(-1),
	_texture2Location(-1),
	_miscValLocation(-1),
	_cfgValLocation(-1),
	_ambientColorLocation(-1)
{
	for (int i = 0; i < 4; i++)
	{
		_litColorLocation[i] = -1;
		_litDirPosLocation[i] = -1;
	}
}

OglProgram::~OglProgram()
{
	if (_program)
		glDeleteProgram(_program);
}

void OglProgram::buildProgram(const std::string& vs, const std::string& ps)
{
	OglShader* vertexShader = (OglShader*) MigUtil::theRend->getShader(vs);
	if (vertexShader == nullptr)
		throw std::invalid_argument("(OglProgram::buildProgram) Vertex shader doesn't exist");
	if (vertexShader->getType() != Shader::SHADER_TYPE_VERTEX)
		throw std::invalid_argument("(OglProgram::buildProgram) Specified shader isn't a vertex shader");

	OglShader* pixelShader = (OglShader*)MigUtil::theRend->getShader(ps);
	if (pixelShader == nullptr)
		throw std::invalid_argument("(OglProgram::buildProgram) Pixel shader doesn't exist");
	if (pixelShader->getType() != Shader::SHADER_TYPE_PIXEL)
		throw std::invalid_argument("(OglProgram::buildProgram) Specified shader isn't a pixel shader");

    _program = glCreateProgram();
    if (_program == 0)
		throw std::runtime_error("(OglProgram::buildProgram) glCreateProgram() failed");

    glAttachShader(_program, vertexShader->getShaderID());
    checkGLError("OglProgram::buildProgram", "glAttachShader");
    glAttachShader(_program, pixelShader->getShaderID());
	checkGLError("OglProgram::buildProgram", "glAttachShader");
    glLinkProgram(_program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(_program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
	{
        GLint bufLength = 0;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength)
		{
            char* buf = (char*) malloc(bufLength);
            if (buf)
			{
				glGetProgramInfoLog(_program, bufLength, nullptr, buf);
                LOGERR("(OglProgram::buildProgram) Could not link program:\n%s\n", buf);
                free(buf);
            }
        }

		glDeleteProgram(_program);
		_program = 0;
		throw std::runtime_error("(OglProgram::buildProgram) Program failed to link");
    }

	// if any of these do not exist, that's ok we just won't update them during rendering
	_gvPositionHandle = glGetAttribLocation(_program, "vPosition");
	_gvColorHandle = glGetAttribLocation(_program, "vColor");
	_gvNormHandle = glGetAttribLocation(_program, "vNorm");
	_gvTex1Handle = glGetAttribLocation(_program, "vTex1");
	_gvTex2Handle = glGetAttribLocation(_program, "vTex2");
	_modelLocation = glGetUniformLocation(_program, "matModel");
	_viewLocation = glGetUniformLocation(_program, "matView");
	_projLocation = glGetUniformLocation(_program, "matProj");
	_mvpLocation = glGetUniformLocation(_program, "matMVP");
	_objColorLocation = glGetUniformLocation(_program, "colObject");
	_texture1Location = glGetUniformLocation(_program, "texture1");
	_texture2Location = glGetUniformLocation(_program, "texture2");
	_miscValLocation = glGetUniformLocation(_program, "miscVal");
	_cfgValLocation = glGetUniformLocation(_program, "cfgVal");
	_ambientColorLocation = glGetUniformLocation(_program, "colAmbient");
	_litColorLocation[0] = glGetUniformLocation(_program, "colLit1");
	_litColorLocation[1] = glGetUniformLocation(_program, "colLit2");
	_litColorLocation[2] = glGetUniformLocation(_program, "colLit3");
	_litColorLocation[3] = glGetUniformLocation(_program, "colLit4");
	_litDirPosLocation[0] = glGetUniformLocation(_program, "dirPosLit1");
	_litDirPosLocation[1] = glGetUniformLocation(_program, "dirPosLit2");
	_litDirPosLocation[2] = glGetUniformLocation(_program, "dirPosLit3");
	_litDirPosLocation[3] = glGetUniformLocation(_program, "dirPosLit4");
}

void OglProgram::useProgram()
{
	glUseProgram(_program);
	//checkGLError("OglProgram::useProgram", "glUseProgram");
}

bool OglProgram::loadVerts(GLfloat* verts)
{
	if (_gvPositionHandle != -1 && verts != nullptr)
	{
		glVertexAttribPointer(_gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, verts);
		//checkGLError("OglProgram::render", "glVertexAttribPointer");
		glEnableVertexAttribArray(_gvPositionHandle);
		//checkGLError("OglProgram::render", "glEnableVertexAttribArray");
		return true;
	}
	return false;
}

bool OglProgram::loadColors(GLfloat* colors)
{
	if (_gvColorHandle != -1 && colors)
	{
		glVertexAttribPointer(_gvColorHandle, 4, GL_FLOAT, GL_FALSE, 0, colors);
		//checkGLError("OglProgram::render", "glVertexAttribPointer");
		glEnableVertexAttribArray(_gvColorHandle);
		//checkGLError("OglProgram::render", "glEnableVertexAttribArray");
		return true;
	}
	return false;
}

bool OglProgram::loadNorms(GLfloat* norms)
{
	if (_gvNormHandle != -1 && norms)
	{
		glVertexAttribPointer(_gvNormHandle, 3, GL_FLOAT, GL_FALSE, 0, norms);
		//checkGLError("OglProgram::render", "glVertexAttribPointer");
		glEnableVertexAttribArray(_gvNormHandle);
		//checkGLError("OglProgram::render", "glEnableVertexAttribArray");
		return true;
	}
	return false;
}

bool OglProgram::loadTex1Coords(GLfloat* tex1)
{
	if (_gvTex1Handle != -1 && tex1)
	{
		glVertexAttribPointer(_gvTex1Handle, 2, GL_FLOAT, GL_FALSE, 0, tex1);
		//checkGLError("OglProgram::render", "glVertexAttribPointer");
		glEnableVertexAttribArray(_gvTex1Handle);
		//checkGLError("OglProgram::render", "glEnableVertexAttribArray");
		return true;
	}
	return false;
}

bool OglProgram::loadTex2Coords(GLfloat* tex2)
{
	if (_gvTex2Handle != -1 && tex2)
	{
		glVertexAttribPointer(_gvTex2Handle, 2, GL_FLOAT, GL_FALSE, 0, tex2);
		//checkGLError("OglProgram::render", "glVertexAttribPointer");
		glEnableVertexAttribArray(_gvTex2Handle);
		//checkGLError("OglProgram::render", "glEnableVertexAttribArray");
		return true;
	}
	return false;
}

bool OglProgram::setTex1Location()
{
	if (_texture1Location != -1)
		glUniform1i(_texture1Location, 0);
	return (_texture1Location != -1);
}

bool OglProgram::setTex2Location()
{
	if (_texture2Location != -1)
		glUniform1i(_texture2Location, 1);
	return (_texture2Location != -1);
}

void OglProgram::loadBasicConfig()
{
	OglRender* rendObj = (OglRender*)MigTech::MigUtil::theRend;
	if (_objColorLocation > -1)
	{
		const Color& objCol = rendObj->getObjectColor();
		glUniform4f(_objColorLocation, objCol.r, objCol.g, objCol.b, objCol.a);
	}
	if (_miscValLocation > -1)
	{
		const float* miscVals = rendObj->getMiscVal();
		//glUniform4fv(_miscValLocation, 4, miscVals);
		glUniform4f(_miscValLocation, miscVals[0], miscVals[1], miscVals[2], miscVals[3]);
	}
	if (_cfgValLocation > -1)
	{
		const int* cfgVals = rendObj->getConfigVal();
		//glUniform4iv(_cfgValLocation, 4, cfgVals);
		glUniform4i(_cfgValLocation, cfgVals[0], cfgVals[1], cfgVals[2], cfgVals[3]);
	}
}

void OglProgram::loadMatrices()
{
	OglRender* rendObj = (OglRender*)MigTech::MigUtil::theRend;
	if (_modelLocation > -1)
		rendObj->loadModelMatrix(_modelLocation);
	if (_viewLocation > -1)
		rendObj->loadViewMatrix(_viewLocation);
	if (_projLocation > -1)
		rendObj->loadProjMatrix(_projLocation);
	if (_mvpLocation > -1)
		rendObj->loadMVPMatrix(_mvpLocation);
}

void OglProgram::loadLights()
{
	OglRender* rendObj = (OglRender*)MigTech::MigUtil::theRend;
	if (_ambientColorLocation > -1)
	{
		const Color& litCol = rendObj->getAmbientColor();
		glUniform4f(_ambientColorLocation, litCol.r, litCol.g, litCol.b, litCol.a);
	}
	for (int i = 0; i < 4; i++)
	{
		if (_litColorLocation[i] > -1)
		{
			const Color& litCol = rendObj->getLightColor(i);
			glUniform4f(_litColorLocation[i], litCol.r, litCol.g, litCol.b, litCol.a);
		}
		if (_litDirPosLocation[i] > -1)
		{
			const Vector3& litDirPos = rendObj->getLightDirPos(i);
			bool isDir = rendObj->getLightIsDir(i);
			glUniform4f(_litDirPosLocation[i], litDirPos.x, litDirPos.y, litDirPos.z, (isDir ? 0 : 1));
		}
	}
}
