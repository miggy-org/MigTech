#include "pch.h"
#include "Matrix.h"
#include "MigUtil.h"

using namespace MigTech;

Matrix::Matrix()
{
	if (MigUtil::theRend != nullptr)
		_mat = MigUtil::theRend->createMatrix();
	else
		_mat = nullptr;
}

Matrix::Matrix(const Matrix& other)
{
	if (MigUtil::theRend != nullptr)
	{
		_mat = MigUtil::theRend->createMatrix();
		_mat->copy(other._mat);
	}
	else
		throw std::runtime_error("(Matrix::Matrix) Cannot copy another matrix w/o a renderer");
}

Matrix::Matrix(IMatrix* passign)
{
	_mat = passign;
}

Matrix::~Matrix()
{
	if (_mat != nullptr && MigUtil::theRend != nullptr)
		MigUtil::theRend->deleteMatrix(_mat);
}

void Matrix::identity()
{
	// special case - delayed creation
	if (_mat == nullptr && MigUtil::theRend != nullptr)
		_mat = MigUtil::theRend->createMatrix();

	_mat->identity();
}

void Matrix::copy(const Matrix& mat)
{
	_mat->copy(mat._mat);
}

void Matrix::load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	_mat->load(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

void Matrix::load(const float* pelem)
{
	_mat->load(pelem);
}

void Matrix::loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ)
{
	_mat->loadPerspectiveFovRH(angleY, aspect, nearZ, farZ);
}

void Matrix::loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector)
{
	_mat->loadLookAtRH(eyePos, focusPos, upVector);
}

void Matrix::multiply(const Matrix& mat)
{
	_mat->multiply(mat._mat);
}

void Matrix::translate(const Vector3& offset)
{
	_mat->translate(offset);
}

void Matrix::translate(float x, float y, float z)
{
	_mat->translate(x, y, z);
}

void Matrix::rotateX(float angle)
{
	_mat->rotateX(angle);
}

void Matrix::rotateY(float angle)
{
	_mat->rotateY(angle);
}

void Matrix::rotateZ(float angle)
{
	_mat->rotateZ(angle);
}

void Matrix::scale(float sx, float sy, float sz)
{
	_mat->scale(sx, sy, sz);
}

void Matrix::transform(Vector3& pt) const
{
	_mat->transform(pt);
}

void Matrix::dump(const char* prefix) const
{
	_mat->dump(prefix);
}

IMatrix* Matrix::operator*(const Matrix& rhs)
{
	IMatrix* pmat = MigUtil::theRend->createMatrix();
	pmat->copy(_mat);
	pmat->multiply(rhs._mat);
	return pmat;
}
