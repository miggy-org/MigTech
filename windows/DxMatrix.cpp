#include "pch.h"
#include "../core/MigUtil.h"
#include "DxMatrix.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;
using namespace DirectX;

void* DxMatrix::operator new(size_t size)
{
	return (DxMatrix*)_aligned_malloc(size, 16);
}

void DxMatrix::operator delete(void* ptr)
{
	_aligned_free(ptr);
}

DxMatrix::DxMatrix()
{
	identity();
}

DxMatrix::~DxMatrix()
{
}

void DxMatrix::identity()
{
	_theMat = XMMatrixIdentity();
	_isIdentity = true;
}

void DxMatrix::copy(const IMatrix* pmat)
{
	if (pmat == nullptr)
		throw std::invalid_argument("(DxMatrix::copy) pmat is nullptr");

	_theMat = (((DxMatrix*)pmat)->_theMat);
	_isIdentity = false;
}

void DxMatrix::load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	XMFLOAT4X4 mat(
		m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33
		);
	_theMat = XMLoadFloat4x4(&mat);
	_isIdentity = false;
}

void DxMatrix::load(const float* pelem)
{
	if (pelem == nullptr)
		throw std::invalid_argument("(DxMatrix::load) pelem is nullptr");

	XMFLOAT4X4 mat(
		pelem[0], pelem[1], pelem[2], pelem[3],
		pelem[4], pelem[5], pelem[6], pelem[7],
		pelem[8], pelem[9], pelem[10], pelem[11],
		pelem[12], pelem[13], pelem[14], pelem[15]
		);
	_theMat = XMLoadFloat4x4(&mat);
	_isIdentity = false;
}

void DxMatrix::loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ)
{
	_theMat = XMMatrixPerspectiveFovRH(angleY, aspect, nearZ, farZ);
	_isIdentity = false;
}

void DxMatrix::loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector)
{
	XMVECTORF32 eye = { eyePos.x, eyePos.y, eyePos.z, 0.0f };
	XMVECTORF32 at = { focusPos.x, focusPos.y, focusPos.z, 0.0f };
	XMVECTORF32 up = { upVector.x, upVector.y, upVector.z, 0.0f };
	_theMat = XMMatrixLookAtRH(eye, at, up);
	_isIdentity = false;
}

void DxMatrix::multiply(const IMatrix* pmat)
{
	if (pmat == nullptr)
		throw std::invalid_argument("(DxMatrix::multiply) pelem is nullptr");

	if (_isIdentity)
		_theMat = (((DxMatrix*)pmat)->_theMat);
	else
		_theMat = XMMatrixMultiply(_theMat, ((DxMatrix*)pmat)->_theMat);
	_isIdentity = false;
}

void DxMatrix::translate(const Vector3& offset)
{
	if (_isIdentity)
		_theMat = XMMatrixTranslation(offset.x, offset.y, offset.z);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixTranslation(offset.x, offset.y, offset.z));
	_isIdentity = false;
}

void DxMatrix::translate(float x, float y, float z)
{
	if (_isIdentity)
		_theMat = XMMatrixTranslation(x, y, z);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixTranslation(x, y, z));
	_isIdentity = false;
}

void DxMatrix::rotateX(float angle)
{
	if (_isIdentity)
		_theMat = XMMatrixRotationX(angle);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixRotationX(angle));
	_isIdentity = false;
}

void DxMatrix::rotateY(float angle)
{
	if (_isIdentity)
		_theMat = XMMatrixRotationY(angle);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixRotationY(angle));
	_isIdentity = false;
}

void DxMatrix::rotateZ(float angle)
{
	if (_isIdentity)
		_theMat = XMMatrixRotationZ(angle);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixRotationZ(angle));
	_isIdentity = false;
}

void DxMatrix::scale(float sx, float sy, float sz)
{
	if (_isIdentity)
		_theMat = XMMatrixScaling(sx, sy, sz);
	else
		_theMat = XMMatrixMultiply(_theMat, XMMatrixScaling(sx, sy, sz));
	_isIdentity = false;
}

void DxMatrix::transform(Vector3& pt) const
{
	XMVECTORF32 v = { pt.x, pt.y, pt.z, 1 };
	XMVECTOR r = XMVector4Transform(v, _theMat);
	pt.x = XMVectorGetX(r);
	pt.y = XMVectorGetY(r);
	pt.z = XMVectorGetZ(r);
}

void DxMatrix::dump(const char* prefix) const
{
	LOGINFO(prefix);
#if defined(_XM_SSE_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
	LOGINFO("%f %f %f %f", _theMat.r[0].m128_f32[0], _theMat.r[0].m128_f32[1], _theMat.r[0].m128_f32[2], _theMat.r[0].m128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[1].m128_f32[0], _theMat.r[1].m128_f32[1], _theMat.r[1].m128_f32[2], _theMat.r[1].m128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[2].m128_f32[0], _theMat.r[2].m128_f32[1], _theMat.r[2].m128_f32[2], _theMat.r[2].m128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[3].m128_f32[0], _theMat.r[3].m128_f32[1], _theMat.r[3].m128_f32[2], _theMat.r[3].m128_f32[3]);
#elif defined(_XM_ARM_NEON_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
	LOGINFO("%f %f %f %f", _theMat.r[0].n128_f32[0], _theMat.r[0].n128_f32[1], _theMat.r[0].n128_f32[2], _theMat.r[0].n128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[1].n128_f32[0], _theMat.r[1].n128_f32[1], _theMat.r[1].n128_f32[2], _theMat.r[1].n128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[2].n128_f32[0], _theMat.r[2].n128_f32[1], _theMat.r[2].n128_f32[2], _theMat.r[2].n128_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[3].n128_f32[0], _theMat.r[3].n128_f32[1], _theMat.r[3].n128_f32[2], _theMat.r[3].n128_f32[3]);
#else
	LOGINFO("%f %f %f %f", _theMat.r[0].vector4_f32[0], _theMat.r[0].vector4_f32[1], _theMat.r[0].vector4_f32[2], _theMat.r[0].vector4_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[1].vector4_f32[0], _theMat.r[1].vector4_f32[1], _theMat.r[1].vector4_f32[2], _theMat.r[1].vector4_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[2].vector4_f32[0], _theMat.r[2].vector4_f32[1], _theMat.r[2].vector4_f32[2], _theMat.r[2].vector4_f32[3]);
	LOGINFO("%f %f %f %f", _theMat.r[3].vector4_f32[0], _theMat.r[3].vector4_f32[1], _theMat.r[3].vector4_f32[2], _theMat.r[3].vector4_f32[3]);
#endif
}
