#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Matrix.h"

namespace MigTech
{
	class DxMatrix : public IMatrix
	{
	public:
		// implemented to get around warning C4316 (due to XMMATRIX below)
		void* operator new(size_t);
		void operator delete(void*);

	protected:
		DirectX::XMMATRIX _theMat;
		bool _isIdentity;

	public:
		DxMatrix();
		virtual ~DxMatrix();

		virtual void identity();
		virtual void copy(const IMatrix* pmat);
		virtual void load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
		virtual void load(const float* pelem);

		virtual void loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ);
		virtual void loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector);

		virtual void multiply(const IMatrix* pmat);
		virtual void translate(const Vector3& offset);
		virtual void translate(float x, float y, float z);
		virtual void rotateX(float angle);
		virtual void rotateY(float angle);
		virtual void rotateZ(float angle);
		virtual void scale(float sx, float sy, float sz);

		virtual void transform(Vector3& pt) const;

		virtual void dump(const char* prefix) const;

	public:
		// used by DxRender only
		DirectX::XMMATRIX getMat() {
			return _theMat;
		}
	};
}
