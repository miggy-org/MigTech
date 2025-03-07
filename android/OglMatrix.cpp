#include "pch.h"
#include "../core/MigUtil.h"
#include "OglMatrix.h"

///////////////////////////////////////////////////////////////////////////
// platform specific (some of this code was copied from http://glm.g-truc.net)

using namespace MigTech;

OglMatrix::OglMatrix()
{
	identity();
}

OglMatrix::~OglMatrix()
{
}

void OglMatrix::identity()
{
	_mat[0]  = 1; _mat[1]  = 0; _mat[2]  = 0; _mat[3]  = 0;
	_mat[4]  = 0; _mat[5]  = 1; _mat[6]  = 0; _mat[7]  = 0;
	_mat[8]  = 0; _mat[9]  = 0; _mat[10] = 1; _mat[11] = 0;
	_mat[12] = 0; _mat[13] = 0; _mat[14] = 0; _mat[15] = 1;
	_isIdentity = true;
}

void OglMatrix::copy(const IMatrix* pmat)
{
	if (pmat == nullptr)
		throw std::invalid_argument("(OglMatrix::copy) pmat is nullptr");
	OglMatrix* pomat = (OglMatrix*) pmat;

	for (int i = 0; i < 16; i++)
		_mat[i] = pomat->_mat[i];
	_isIdentity = false;
}

void OglMatrix::load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	_mat[0]  = m00; _mat[1]  = m10; _mat[2]  = m20; _mat[3]  = m30;
	_mat[4]  = m01; _mat[5]  = m11; _mat[6]  = m21; _mat[7]  = m31;
	_mat[8]  = m02; _mat[9]  = m12; _mat[10] = m22; _mat[11] = m32;
	_mat[12] = m03; _mat[13] = m13; _mat[14] = m23; _mat[15] = m33;
	_isIdentity = false;
}

void OglMatrix::load(const float* pelem)
{
	if (pelem == nullptr)
		throw std::invalid_argument("(OglMatrix::load) pelem is nullptr");

	_mat[0]  = pelem[0]; _mat[1]  = pelem[4]; _mat[2]  = pelem[8]; _mat[3]  = pelem[12];
	_mat[4]  = pelem[1]; _mat[5]  = pelem[5]; _mat[6]  = pelem[9]; _mat[7]  = pelem[13];
	_mat[8]  = pelem[2]; _mat[9]  = pelem[6]; _mat[10] = pelem[10]; _mat[11] = pelem[14];
	_mat[12] = pelem[3]; _mat[13] = pelem[7]; _mat[14] = pelem[11]; _mat[15] = pelem[15];
	_isIdentity = false;
}

// copied from OpenGL GLM code
void OglMatrix::loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ)
{
	float tanHalfFovy = tan(angleY / static_cast<float>(2));

	_mat[0] = static_cast<float>(1) / (aspect * tanHalfFovy);
	_mat[5] = static_cast<float>(1) / (tanHalfFovy);
	_mat[10] = -(farZ + nearZ) / (farZ - nearZ);
	_mat[11] = -static_cast<float>(1);
	_mat[14] = -(static_cast<float>(2) * farZ * nearZ) / (farZ - nearZ);
	_mat[15] = 0;
	_isIdentity = false;
}

#define normalize(x, y, z)                  \
{                                           \
    float norm = 1.0f / sqrt(x*x+y*y+z*z);  \
    x *= norm; y *= norm; z *= norm;        \
}

// copied from OpenGL GLM code
void OglMatrix::loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector)
{
	float fx = focusPos.x - eyePos.x;
	float fy = focusPos.y - eyePos.y;
	float fz = focusPos.z - eyePos.z;
	normalize(fx, fy, fz);
	float sx = fy * upVector.z - fz * upVector.y;
	float sy = fz * upVector.x - fx * upVector.z;
	float sz = fx * upVector.y - fy * upVector.x;
	normalize(sx, sy, sz);
	float ux = sy * fz - sz * fy;
	float uy = sz * fx - sx * fz;
	float uz = sx * fy - sy * fx;

	_mat[ 0] = sx;
	_mat[ 1] = ux;
	_mat[ 2] = -fx;
	_mat[ 3] = 0.0f;
	_mat[ 4] = sy;
	_mat[ 5] = uy;
	_mat[ 6] = -fy;
	_mat[ 7] = 0.0f;
	_mat[ 8] = sz;
	_mat[ 9] = uz;
	_mat[10] = -fz;
	_mat[11] = 0.0f;
	_mat[12] = -(sx*eyePos.x + sy*eyePos.y + sz*eyePos.z);
	_mat[13] = -(ux*eyePos.x + uy*eyePos.y + uz*eyePos.z);
	_mat[14] =  (fx*eyePos.x + fy*eyePos.y + fz*eyePos.z);
	_mat[15] = 1.0f;
	_isIdentity = false;
}

void OglMatrix::multiply(const IMatrix* pmat)
{
	if (pmat == nullptr)
		throw std::invalid_argument("(OglMatrix::multiply) pelem is nullptr");
	OglMatrix* pomat = (OglMatrix*) pmat;

	if (!_isIdentity)
	{
		GLfloat ctm[16];
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				ctm[4 * j + i] = 0;

				for (int k = 0; k < 4; k++)
				{
					//ctm[4*j+i] += _mat[4*k+i]*pomat->_mat[4*j+k];
					ctm[4 * j + i] += pomat->_mat[4 * k + i] * _mat[4 * j + k];
				}
			}
		}
		memcpy(_mat, ctm, 16 * sizeof(GLfloat));
	}
	else
		copy(pmat);

	_isIdentity = false;
}

void OglMatrix::translate(const Vector3& offset)
{
	if (_isIdentity)
	{
		_mat[12] = offset.x;
		_mat[13] = offset.y;
		_mat[14] = offset.z;
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[12] = offset.x;
		tmp._mat[13] = offset.y;
		tmp._mat[14] = offset.z;
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::translate(float x, float y, float z)
{
	if (_isIdentity)
	{
		_mat[12] = x;
		_mat[13] = y;
		_mat[14] = z;
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[12] = x;
		tmp._mat[13] = y;
		tmp._mat[14] = z;
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::rotateX(float angle)
{
	if (_isIdentity)
	{
		_mat[5] = cos(angle);
		_mat[6] = sin(angle);
		_mat[9] = -sin(angle);
		_mat[10] = cos(angle);
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[5] = cos(angle);
		tmp._mat[6] = sin(angle);
		tmp._mat[9] = -sin(angle);
		tmp._mat[10] = cos(angle);
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::rotateY(float angle)
{
	if (_isIdentity)
	{
		_mat[0] = cos(angle);
		_mat[2] = -sin(angle);
		_mat[8] = sin(angle);
		_mat[10] = cos(angle);
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[0] = cos(angle);
		tmp._mat[2] = -sin(angle);
		tmp._mat[8] = sin(angle);
		tmp._mat[10] = cos(angle);
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::rotateZ(float angle)
{
	if (_isIdentity)
	{
		_mat[0] = cos(angle);
		_mat[1] = sin(angle);
		_mat[4] = -sin(angle);
		_mat[5] = cos(angle);
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[0] = cos(angle);
		tmp._mat[1] = sin(angle);
		tmp._mat[4] = -sin(angle);
		tmp._mat[5] = cos(angle);
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::scale(float sx, float sy, float sz)
{
	if (_isIdentity)
	{
		_mat[0] = sx;
		_mat[5] = sy;
		_mat[10] = sz;
	}
	else
	{
		OglMatrix tmp;
		tmp._mat[0] = sx;
		tmp._mat[5] = sy;
		tmp._mat[10] = sz;
		multiply(&tmp);
	}
	//tmp.multiply(this);
	//copy(&tmp);
	_isIdentity = false;
}

void OglMatrix::transform(Vector3& pt) const
{
	// TODO: unsure about this
	Vector3 ptOut;
	ptOut.x = pt.x * _mat[0] + pt.y * _mat[4] + pt.z * _mat[8] + _mat[12];
	ptOut.y = pt.x * _mat[1] + pt.y * _mat[5] + pt.z * _mat[9] + _mat[13];
	ptOut.z = pt.x * _mat[2] + pt.y * _mat[6] + pt.z * _mat[10] + _mat[14];
	pt = ptOut;
}

void OglMatrix::dump(const char* prefix) const
{
	LOGINFO(prefix);
	LOGINFO("%f %f %f %f", _mat[0], _mat[1], _mat[2], _mat[3]);
	LOGINFO("%f %f %f %f", _mat[4], _mat[5], _mat[6], _mat[7]);
	LOGINFO("%f %f %f %f", _mat[8], _mat[9], _mat[10], _mat[11]);
	LOGINFO("%f %f %f %f", _mat[12], _mat[13], _mat[14], _mat[15]);
}
