#pragma once

#include "MigBase.h"
#include "Object.h"
#include "Matrix.h"

namespace MigTech
{
	// text justification
	enum JUSTIFY
	{
		JUSTIFY_IGNORE,
		JUSTIFY_LEFT,
		JUSTIFY_CENTER,
		JUSTIFY_RIGHT
	};

	// stores information in the font about a single character
	struct CharInfo
	{
		char thisChar;
		int itemIndex;
		int left;
		int top;
		int width;
		//int xCenter;
		//int yCenter;
		float uMin, vMin;
		float uMax, vMax;
	};

	// defined below Font
	class Text;

	// TODO: proper UTF8 support (this currently only works w/ ANSI chars)
	class Font : public MigBase
	{
	public:
		Font(const std::string& resID, const std::string& xmlID);
		Font(const std::string& fontCfgName);
		virtual ~Font();

		void create();
		void createGraphics();
		void destroyGraphics();
		void destroy();

		// allows for external configuration of some settings
		void config(float stretch);

		Text* createText() const;
		void destroyText(Text* text) const;

		float getWidth(const std::string& text) const;
		float getHeight(const std::string& text) const;

		void draw(const std::string& text, const Color& col, const Matrix& mat) const;
		void draw(const std::string& text, float r, float g, float b, float a, const Matrix& mat) const;

	protected:
		bool fillRestOfInfo(CharInfo& charInfo) const;
		bool loadConfig();
		float drawChar(char key, float x, float y, float z, const Matrix& mat) const;

	protected:
		std::string _resID;
		std::string _xmlID;

		// loaded texture properties
		int _texWidth;
		int _texHeight;

		// font properties (loaded from XML configuration)
		std::string _fontName;
		int _globalWidth;
		int _globalHeight;
		int _spacing;
		bool _addAlpha;
		bool _useAlpha;
		bool _dropColor;
		float _stretch;
		std::map<char, CharInfo> _charInfoMap;

		// render object
		Object* _fontMapObj;
	};

	// holds a single piece of text ready for rendering
	class Text
	{
	public:
		Text();
		Text(const Font* font);
		~Text();

		void init(const std::string& text, float u, float v, float h, float stretch, JUSTIFY justification = JUSTIFY_IGNORE);
		void init(const std::string& text, float u, float v, float h, JUSTIFY justification = JUSTIFY_IGNORE);
		void init(const std::string& text, JUSTIFY justification = JUSTIFY_IGNORE);
		void update(const std::string& text, JUSTIFY justification = JUSTIFY_IGNORE);
		void transform(float scale, float stretch, float x, float y, float z, float rx, float ry, float rz);
		void destroy();

		// note that these are only meaningful when rendering text in 2D
		float getScreenWidth() const;
		float getScreenHeight() const;
		Rect getScreenRect() const;
		const std::string& getText() const { return _text; }

		void draw(const Color& col) const;
		void draw(const Color& col, const Matrix& worldMatrix) const;
		void draw(float r, float g, float b, float a) const;
		void draw(float r, float g, float b, float a, const Matrix& worldMatrix) const;

	protected:
		const Font* _font;
		std::string _text;
		Vector3 _pt;
		float _scale;
		float _stretch;
		float _offset;
		float _rotX, _rotY, _rotZ;
		float _u, _v, _h;	// 2D only (screen coordinates)
		JUSTIFY _justify;
		Matrix _mat;
	};
}