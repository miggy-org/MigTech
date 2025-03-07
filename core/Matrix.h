#pragma once

#include "MigDefines.h"

namespace MigTech
{
	// this is a matrix interface class that platforms can provide
	class IMatrix
	{
	public:
		IMatrix() { }
		virtual ~IMatrix() { }

		virtual void identity() = 0;
		virtual void copy(const IMatrix* pmat) = 0;
		virtual void load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33) = 0;
		virtual void load(const float* pelem) = 0;

		virtual void loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ) = 0;
		virtual void loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector) = 0;

		virtual void multiply(const IMatrix* pmat) = 0;
		virtual void translate(const Vector3& offset) = 0;
		virtual void translate(float x, float y, float z) = 0;
		virtual void rotateX(float angle) = 0;
		virtual void rotateY(float angle) = 0;
		virtual void rotateZ(float angle) = 0;
		virtual void scale(float sx, float sy, float sz) = 0;

		virtual void transform(Vector3& pt) const = 0;

		virtual void dump(const char* prefix) const = 0;
	};

	// this is simply a container class for IMatrix
	class Matrix
	{
	protected:
		IMatrix* _mat;

	public:
		Matrix();
		Matrix(const Matrix& other);
		Matrix(IMatrix* passign);
		~Matrix();

		void identity();
		void copy(const Matrix& mat);
		void load(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
		void load(const float* pelem);

		void loadPerspectiveFovRH(float angleY, float aspect, float nearZ, float farZ);
		void loadLookAtRH(Vector3 eyePos, Vector3 focusPos, Vector3 upVector);

		void multiply(const Matrix& mat);
		void translate(const Vector3& offset);
		void translate(float x, float y, float z);
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);
		void scale(float sx, float sy, float sz);

		void transform(Vector3& pt) const;

		void dump(const char* prefix) const;

		IMatrix* operator*(const Matrix& rhs);
		//operator IMatrix*() { return _mat; }
		operator const IMatrix*() const { return _mat; }
	};
}
