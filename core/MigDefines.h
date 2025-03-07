#pragma once

namespace MigTech
{
	///////////////////////////////////////////////////////////////////////////
	// defines

// shader hints
#define SHADER_HINT_NONE		0x0
#define SHADER_HINT_MODEL		0x1
#define SHADER_HINT_VIEW		0x2
#define SHADER_HINT_PROJ		0x4
#define SHADER_HINT_MVP			0x8
#define SHADER_HINT_LIGHTS		0x10

// maximum number of supported texture maps per object
#define MAX_TEXTURE_MAPS		2

// image processing that can be performed when loading images
#define LOAD_IMAGE_NONE			0x0
#define LOAD_IMAGE_ADD_ALPHA	0x1
#define LOAD_IMAGE_INVERT_ALPHA	0x2
#define LOAD_IMAGE_SET_ALPHA	0x4
#define LOAD_IMAGE_CLEAR_ALPHA	0x8
#define LOAD_IMAGE_DROP_COLOR	0x10

// platform bits
#define PLAT_WINDOWS			0x1
#define PLAT_ANDROID			0x2
#define PLAT_DESKTOP			0x4
#define PLAT_IOS				0x8

	///////////////////////////////////////////////////////////////////////////
	// enums

	enum ELEM_FORMAT
	{
		ELEM_FORMAT_UNKNOWN,
		ELEM_FORMAT_FLOAT_4X,
		ELEM_FORMAT_FLOAT_3X,
		ELEM_FORMAT_FLOAT_2X,
		ELEM_FORMAT_FLOAT
	};

	enum INPUT_CLASS
	{
		INPUT_CLASS_PER_VERTEX,
		INPUT_CLASS_PER_INSTANCE
	};

	enum VDTYPE
	{
		VDTYPE_UNKNOWN,
		VDTYPE_POSITION,
		VDTYPE_POSITION_COLOR,
		VDTYPE_POSITION_COLOR_TEXTURE,
		VDTYPE_POSITION_NORMAL,
		VDTYPE_POSITION_NORMAL_TEXTURE,
		VDTYPE_POSITION_TEXTURE,
		VDTYPE_POSITION_TEXTURE_TEXTURE
	};

	enum PRIMITIVE_TYPE
	{
		PRIMITIVE_TYPE_UNKNOWN,
		PRIMITIVE_TYPE_TRIANGLE_LIST,
		PRIMITIVE_TYPE_TRIANGLE_STRIP,
		PRIMITIVE_TYPE_TRIANGLE_FAN
	};

	enum FACE_CULLING
	{
		FACE_CULLING_NONE,
		FACE_CULLING_FRONT,
		FACE_CULLING_BACK
	};

	enum BLEND_STATE
	{
		BLEND_STATE_NONE,
		BLEND_STATE_SRC_ALPHA,
		BLEND_STATE_ONE_MINUS_SRC_ALPHA,
		BLEND_STATE_DST_ALPHA,
		BLEND_STATE_ONE_MINUS_DST_ALPHA
	};

	enum DEPTH_TEST_STATE
	{
		DEPTH_TEST_STATE_NONE,
		DEPTH_TEST_STATE_LESS,
		DEPTH_TEST_STATE_LEQUAL,
		DEPTH_TEST_STATE_EQUAL,
		DEPTH_TEST_STATE_GEQUAL,
		DEPTH_TEST_STATE_GREATER,
		DEPTH_TEST_STATE_NEQUAL,
		DEPTH_TEST_STATE_ALWAYS
	};

	enum IMG_FORMAT
	{
		IMG_FORMAT_NONE,
		IMG_FORMAT_GREYSCALE,
		IMG_FORMAT_ALPHA,
		IMG_FORMAT_RGB,
		IMG_FORMAT_RGBA
	};

	enum TXT_FILTER
	{
		TXT_FILTER_NONE,
		TXT_FILTER_NEAREST,
		TXT_FILTER_LINEAR,
		TXT_FILTER_NEAREST_MIPMAP_NEAREST,
		TXT_FILTER_LINEAR_MIPMAP_NEAREST,
		TXT_FILTER_NEAREST_MIPMAP_LINEAR,
		TXT_FILTER_LINEAR_MIPMAP_LINEAR
	};

	enum TXT_WRAP
	{
		TXT_WRAP_NONE,
		TXT_WRAP_CLAMP,
		TXT_WRAP_REPEAT,
		TXT_WRAP_MIRRORED_REPEAT
	};

	enum VIRTUAL_KEY
	{
		// note that these match Windows::System::VirtualKey
		NONE = 0,
		BACK = 8,
		TAB = 9,
		CLEAR = 12,
		ENTER = 13,
		SHIFT = 16,
		CONTROL = 17,
		ESCAPE = 27,
		SPACE = 32,
		PAGEUP = 33,
		PAGEDOWN = 34,
		END = 35,
		HOME = 36,
		LEFT = 37,
		UP = 38,
		RIGHT = 39,
		DOWN = 40,
		INSERT = 45,
		DEL = 46,
		NUMPAD0 = 96,
		NUMPAD1 = 97,
		NUMPAD2 = 98,
		NUMPAD3 = 99,
		NUMPAD4 = 100,
		NUMPAD5 = 101,
		NUMPAD6 = 102,
		NUMPAD7 = 103,
		NUMPAD8 = 104,
		NUMPAD9 = 105
	};

	enum SWIPE_STYLE
	{
		SWIPE_NONE = 0,
		SWIPE_HORIZONTAL,
		SWIPE_VERTICAL
	};

	///////////////////////////////////////////////////////////////////////////
	// common structs

	struct Size
	{
		float width;
		float height;

		Size() { width = height = 0; }
		Size(float w, float h) { width = w; height = h; }

		bool operator == (const Size& other) { return (width == other.width && height == other.height ? true : false); }
		bool operator != (const Size& other) { return (width == other.width && height == other.height ? false : true); }
	};

	struct Vector2
	{
		float x;
		float y;

		Vector2() { x = y = 0; }
		Vector2(float _x, float _y) { x = _x; y = _y; }

		const Vector2& normalize()
		{
			float len = (float) sqrt(x*x + y*y);
			if (len > 0) { x /= len; y /= len; }
			return *this;
		}

		bool operator == (const Vector2& other) { return (x == other.x && y == other.y ? true : false); }
		bool operator != (const Vector2& other) { return (x == other.x && y == other.y ? false : true); }
		Vector2 operator + (const Vector2& rhs) { return Vector2(x + rhs.x, y + rhs.y); }
		Vector2 operator - (const Vector2& rhs) { return Vector2(x - rhs.x, y - rhs.y); }
		Vector2 operator * (float scale) { return Vector2(x * scale, y * scale); }
		void operator += (const Vector2& rhs) { x += rhs.x; y += rhs.y; }
		void operator -= (const Vector2& rhs) { x -= rhs.x; y -= rhs.y; }
		void operator *= (float scale) { x *= scale; y *= scale; }
	};

	struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3() { x = y = z = 0; }
		Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

		const Vector3& normalize()
		{
			float len = (float) sqrt(x*x + y*y + z*z);
			if (len > 0) { x /= len; y /= len; z /= len; }
			return *this;
		}

		bool operator == (const Vector3& other) { return (x == other.x && y == other.y && z == other.z ? true : false); }
		bool operator != (const Vector3& other) { return (x == other.x && y == other.y && z == other.z ? false : true); }
		Vector3 operator + (const Vector3& rhs) { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
		Vector3 operator - (const Vector3& rhs) { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
		Vector3 operator * (float scale) { return Vector3(x * scale, y * scale, z * scale); }
		void operator += (const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }
		void operator -= (const Vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }
		void operator *= (float scale) { x *= scale; y *= scale; z *= scale; }
	};

	struct Color
	{
		float r;
		float g;
		float b;
		float a;

		Color() { r = g = b = a = 0; }
		Color(float _c) { r = _c; g = _c; b = _c; a = 1; }
		Color(float _c, float _a) { r = _c; g = _c; b = _c; a = _a; }
		Color(float _r, float _g, float _b) { r = _r; g = _g; b = _b; a = 1; }
		Color(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }
		Color(const Color& _col, float _a) { r = _col.r; g = _col.g; b = _col.b; a = _a; }

		bool operator == (const Color& other) { return (r == other.r && g == other.g && b == other.b && a == other.a ? true : false); }
		bool operator != (const Color& other) { return (r == other.r && g == other.g && b == other.b && a == other.a ? false : true); }
	};

	struct Rect
	{
		Vector2 corner;
		Size size;

		Rect() { Empty(); }
		Rect(Vector2 c, Size s) { corner = c; size = s; }
		Rect(float x, float y, float w, float h) { corner.x = x; corner.y = y; size.width = w; size.height = h; }

		void Empty() { corner.x = corner.y = 0; size.width = size.height = 0; }
		bool IsEmpty() const { return (size.width == 0 && size.height == 0); }
		void Inflate(float dx, float dy) { corner.x -= dx; corner.y -= dy; size.width += 2 * dx; size.height += 2 * dy; }
		bool Contains(float x, float y) const { return (x >= corner.x && y >= corner.y && x < (corner.x + size.width) && y < (corner.y + size.height)); }

		static Rect Union(const Rect& r1, const Rect& r2)
		{
			return Rect(
				(float)fmin(r1.corner.x, r2.corner.x),
				(float)fmin(r1.corner.y, r2.corner.y),
				(float)(fmax(r1.corner.x + r1.size.width, r2.corner.x + r2.size.width) - fmin(r1.corner.x, r2.corner.x)),
				(float)(fmax(r1.corner.y + r1.size.height, r2.corner.y + r2.size.height) - fmin(r1.corner.y, r2.corner.y)));
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// common vertex structures used to define geometry (one for each VDTYPE)

	// VDTYPE_POSITION
	struct VertexPosition
	{
		Vector3 pos;

		VertexPosition() { }
		VertexPosition(const Vector3& p) { pos = p; }
	};

	// VDTYPE_POSITION_COLOR
	struct VertexPositionColor
	{
		Vector3 pos;
		Color color;

		VertexPositionColor() { }
		VertexPositionColor(const Vector3& p, const Color& c) { pos = p; color = c; }
	};

	// VDTYPE_POSITION_COLOR_TEXTURE
	struct VertexPositionColorTexture
	{
		Vector3 pos;
		Color color;
		Vector2 uv;

		VertexPositionColorTexture() { }
		VertexPositionColorTexture(const Vector3& p, const Color& c, const Vector2& t) { pos = p; color = c; uv = t; }
	};

	// VDTYPE_POSITION_NORMAL
	struct VertexPositionNormal
	{
		Vector3 pos;
		Vector3 norm;

		VertexPositionNormal() { }
		VertexPositionNormal(const Vector3& p, const Vector3& n) { pos = p; norm = n; }
	};

	// VDTYPE_POSITION_NORMAL_TEXTURE
	struct VertexPositionNormalTexture
	{
		Vector3 pos;
		Vector3 norm;
		Vector2 uv;

		VertexPositionNormalTexture() { }
		VertexPositionNormalTexture(const Vector3& p, const Vector3& n, const Vector2& t) { pos = p; norm = n; uv = t; }
	};

	// VDTYPE_POSITION_TEXTURE
	struct VertexPositionTexture
	{
		Vector3 pos;
		Vector2 uv;

		VertexPositionTexture() { }
		VertexPositionTexture(const Vector3& p, const Vector2& t) { pos = p; uv = t; }
	};

	// VDTYPE_POSITION_TEXTURE_TEXTURE
	struct VertexPositionTextureTexture
	{
		Vector3 pos;
		Vector2 uv1;
		Vector2 uv2;

		VertexPositionTextureTexture() { }
		VertexPositionTextureTexture(const Vector3& p, const Vector2& t1, const Vector2& t2) { pos = p; uv1 = t1; uv2 = t2; }
	};
}