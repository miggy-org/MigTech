#include "pch.h"
#include "Font.h"
#include "MigUtil.h"

using namespace MigTech;
using namespace tinyxml2;

Font::Font(const std::string& resID, const std::string& xmlID) :
	_resID(resID), _xmlID(xmlID),
	_texWidth(0), _texHeight(0),
	_globalWidth(0), _globalHeight(0),
	_spacing(0),
	_addAlpha(false), _useAlpha(false), _dropColor(false),
	_stretch(1), _fontMapObj(nullptr)
{
}

Font::Font(const std::string& fontCfgName) :
	_texWidth(0), _texHeight(0),
	_globalWidth(0), _globalHeight(0),
	_spacing(0),
	_addAlpha(false), _useAlpha(false), _dropColor(false),
	_stretch(1), _fontMapObj(nullptr)
{
	const XMLElement* elem = MigUtil::theGame->getConfigRoot();
	elem = elem->FirstChildElement("fonts");
	if (elem != nullptr)
	{
		elem = elem->FirstChildElement(fontCfgName.c_str());
		if (elem != nullptr)
		{
			_resID = elem->Attribute("image");
			_xmlID = elem->Attribute("xml");
		}
		else
			throw std::invalid_argument("(Font::Font) Specified font doesn't exist in the font configuration block");
	}
	else
		throw std::invalid_argument("(Font::Font) Font configuration block missing");
}

Font::~Font()
{
}

bool Font::fillRestOfInfo(CharInfo& charInfo) const
{
	// the width may be 0, meaning it just inherits the global width
	if (charInfo.width == 0)
		charInfo.width = _globalWidth;

	// compute the u,v corner coordinates
	charInfo.uMin = charInfo.left / (float)_texWidth;
	charInfo.vMin = charInfo.top / (float)_texHeight;
	charInfo.uMax = (charInfo.left + charInfo.width) / (float)_texWidth;
	charInfo.vMax = (charInfo.top + _globalHeight) / (float)_texHeight;

	return true;
}

bool Font::loadConfig()
{
	LOGINFO("(Font::loadConfig) Loading font XML script '%s'", _xmlID.c_str());

	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument(_xmlID);
	if (pdoc != nullptr)
	{
		XMLElement* elem = pdoc->FirstChildElement("Font");
		if (elem != nullptr)
		{
			int currItemIndex = 0;

			// basic font attributes
			_fontName = elem->Attribute("Name");
			_globalWidth = atoi(elem->Attribute("Width"));
			_globalHeight = atoi(elem->Attribute("Height"));
			_spacing = atoi(elem->Attribute("Spacing"));
			_useAlpha = (!strcmp(elem->Attribute("UseAlpha"), "true") ? true : false);
			_addAlpha = (!strcmp(elem->Attribute("AddAlpha"), "true") ? true : false);
			_dropColor = (!strcmp(elem->Attribute("DropColor"), "true") ? true : false);
			_stretch = MigUtil::parseFloat(elem->Attribute("Stretch"), _stretch);
			//LOGDBG("Font name is %s", _fontName.c_str());

			XMLElement* item = elem->FirstChildElement("Item");
			while (item != nullptr)
			{
				CharInfo newCharInfo;
				newCharInfo.thisChar = item->Attribute("Char")[0];
				newCharInfo.itemIndex = currItemIndex++;

				// width is optional
				const char* attr = item->Attribute("Left");
				newCharInfo.left = (attr != nullptr ? atoi(attr) : 0);
				attr = item->Attribute("Top");
				newCharInfo.top = (attr != nullptr ? atoi(attr) : 0);
				attr = item->Attribute("Width");
				newCharInfo.width = (attr != nullptr ? atoi(attr) : 0);
				//LOGDBG("Char is %s, width is %d", newCharInfo.thisChar.c_str(), newCharInfo.width);

				_charInfoMap[newCharInfo.thisChar] = newCharInfo;
				item = item->NextSiblingElement("Item");
			}

			LOGDBG("Done reading font XML script");
		}
		else
			throw std::runtime_error("(Font::create) Root font element not found");

		delete pdoc;
	}
	else
		throw std::invalid_argument("(Font::create) Unable to open font XML script");

	return true;
}

void Font::create()
{
	if (_charInfoMap.size() == 0)
		loadConfig();
}

void Font::createGraphics()
{
	unsigned int loadFlags = LOAD_IMAGE_NONE;
	if (_addAlpha)
		loadFlags |= LOAD_IMAGE_ADD_ALPHA;
	if (_dropColor)
		loadFlags |= LOAD_IMAGE_DROP_COLOR;

	// load the texture map
	Image* ptxt = MigUtil::theRend->loadImage(_resID, _resID, loadFlags);
	if (ptxt != nullptr)
	{
		_texWidth = ptxt->getWidth();
		_texHeight = ptxt->getHeight();

		// create the texture object and assign the shaders
		_fontMapObj = MigUtil::theRend->createObject();
		_fontMapObj->addShaderSet(MIGTECH_VSHADER_POS_TEX_TRANSFORM, (_dropColor ? MIGTECH_PSHADER_TEX_ALPHA : MIGTECH_PSHADER_TEX));

		// create the vertex / index arrays
		int numChars = _charInfoMap.size();
		VertexPositionTexture* verts = new VertexPositionTexture[4*numChars];
		unsigned short* txtIndices = new unsigned short[6*numChars];

		// fill in the arrays
		std::map<char, CharInfo>::iterator iter = _charInfoMap.begin();
		while (iter != _charInfoMap.end())
		{
			CharInfo& info = iter->second;
			int currChar = info.itemIndex;

			// now we can complete the configuration (need texture width/height first)
			fillRestOfInfo(info);

			// load mesh vertices (position / color)
			verts[4 * currChar + 0] = VertexPositionTexture(Vector3(0, 0, 0), Vector2(info.uMin, info.vMax));
			verts[4 * currChar + 1] = VertexPositionTexture(Vector3(0, 1, 0), Vector2(info.uMin, info.vMin));
			verts[4 * currChar + 2] = VertexPositionTexture(Vector3(1, 0, 0), Vector2(info.uMax, info.vMax));
			verts[4 * currChar + 3] = VertexPositionTexture(Vector3(1, 1, 0), Vector2(info.uMax, info.vMin));

			// load mesh indices
			txtIndices[6 * currChar + 0] = 4 * currChar + 0;
			txtIndices[6 * currChar + 1] = 4 * currChar + 2;
			txtIndices[6 * currChar + 2] = 4 * currChar + 1;
			txtIndices[6 * currChar + 3] = 4 * currChar + 1;
			txtIndices[6 * currChar + 4] = 4 * currChar + 2;
			txtIndices[6 * currChar + 5] = 4 * currChar + 3;

			iter++;
		}

		// assign the buffers
		_fontMapObj->loadVertexBuffer(verts, 4 * numChars, MigTech::VDTYPE_POSITION_TEXTURE);
		_fontMapObj->loadIndexBuffer(txtIndices, 6 * numChars, MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

		// texturing / culling
		_fontMapObj->setImage(0, _resID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
		_fontMapObj->setCulling(FACE_CULLING_BACK);
	}
	else
		throw std::runtime_error("(Font::createGraphics) Unable to load font texture");
}

void Font::destroyGraphics()
{
	if (_fontMapObj != nullptr)
		MigUtil::theRend->deleteObject(_fontMapObj);
	_fontMapObj = nullptr;

	MigUtil::theRend->unloadImage(_resID);
}

void Font::destroy()
{
}

void Font::config(float stretch)
{
	_stretch = stretch;
}

Text* Font::createText() const
{
	return new Text(this);
}

void Font::destroyText(Text* text) const
{
	delete text;
}

float Font::getWidth(const std::string& text) const
{
	float width = 0;
	int lenText = text.length();
	for (int i = 0; i < lenText; i++)
	{
		char entry = text[i];
		std::map<char, CharInfo>::const_iterator iter = _charInfoMap.find(entry);
		if (iter != _charInfoMap.end())
		{
			const CharInfo& info = iter->second;
			width += (info.width + _spacing);
		}
	}

	return _stretch * width / (float)_globalWidth;
}

float Font::getHeight(const std::string& text) const
{
	// for now all fonts use a universal height
	return 1;
}

float Font::drawChar(char key, float x, float y, float z, const Matrix& mat) const
{
	float width = 0;

	std::map<char, CharInfo>::const_iterator iter = _charInfoMap.find(key);
	if (iter != _charInfoMap.end())
	{
		// note that this isn't thread safe
		static Matrix localMat;

		const CharInfo& info = iter->second;
		float xScale = _stretch * info.width / (float)_globalWidth;
		localMat.identity();
		localMat.scale(xScale, 1, 1);
		localMat.translate(x, y, z);
		localMat.multiply(mat);
		MigUtil::theRend->setModelMatrix(localMat);

		_fontMapObj->setIndexOffset(6*info.itemIndex, 6);
		_fontMapObj->render();

		width = _stretch * ((info.width + _spacing) / (float)_globalWidth);
	}

	return width;
}

void Font::draw(const std::string& text, const Color& col, const Matrix& mat) const
{
	draw(text, col.r, col.g, col.b, col.a, mat);
}

void Font::draw(const std::string& text, float r, float g, float b, float a, const Matrix& mat) const
{
	if (_fontMapObj == nullptr)
		throw std::runtime_error("(Font::draw) Font rendering object cannot be null");

	MigUtil::theRend->setObjectColor(Color(r, g, b, a));
	MigUtil::theRend->setBlending(_useAlpha ? BLEND_STATE_SRC_ALPHA : BLEND_STATE_NONE);
	MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

	// start a render set (more efficient)
	_fontMapObj->startRenderSet();

	// keep track of the x-offset as we output characters
	float x = 0;
	int lenText = text.length();
	for (int i = 0; i < lenText; i++)
	{
		x += drawChar(text[i], x, 0, 0, mat);
	}

	// finish the render set
	_fontMapObj->stopRenderSet();
}

Text::Text() :
	_font(MigUtil::theFont), _scale(1), _stretch(1), _offset(0),
	_rotX(0), _rotY(0), _rotZ(0),
	_u(0), _v(0), _h(1), _justify(JUSTIFY_IGNORE)
{
	if (_font == nullptr)
		throw std::invalid_argument("(Text::Text) Global font cannot be null");
}

Text::Text(const Font* font) :
	_font(font), _scale(1), _stretch(1), _offset(0),
	_rotX(0), _rotY(0), _rotZ(0),
	_u(0), _v(0), _h(1), _justify(JUSTIFY_IGNORE)
{
	if (_font == nullptr)
		throw std::invalid_argument("(Text::Text) Font argument cannot be null");
}

Text::~Text()
{
}

void Text::init(const std::string& text, float u, float v, float h, float stretch, JUSTIFY justification)
{
	_pt = MigUtil::screenPercentToCameraPlane(u, v);
	_u = u; _v = v;

	_scale = MigUtil::screenPercentHeightToCameraPlane(h);
	_h = h;
	_stretch = stretch;

	update(text, justification);
}

void Text::init(const std::string& text, float u, float v, float h, JUSTIFY justification)
{
	init(text, u, v, h, 1.0f, justification);
}

void Text::init(const std::string& text, JUSTIFY justification)
{
	_pt = Vector3(0, 0, 0);
	_rotX = _rotY = _rotZ = 0;
	_scale = 1;

	update(text, justification);
}

void Text::update(const std::string& text, JUSTIFY justification)
{
	_text = text;
	_justify = justification;

	if (_justify != JUSTIFY_IGNORE)
	{
		_offset = 0;
		if (_justify == JUSTIFY_CENTER)
			_offset += _stretch * _scale * _font->getWidth(text) / 2;
		else if (_justify == JUSTIFY_RIGHT)
			_offset += _stretch * _scale * _font->getWidth(text);
	}

	_mat.identity();
	_mat.scale(_stretch*_scale, _scale, 1);
	_mat.translate(_pt.x - _offset, _pt.y, _pt.z);
	_mat.rotateX(_rotX);
	_mat.rotateY(_rotY);
	_mat.rotateZ(_rotZ);
}

void Text::transform(float scale, float stretch, float x, float y, float z, float rx, float ry, float rz)
{
	_pt = Vector3(x, y, z);
	_rotX = rx; _rotY = ry; _rotZ = rz;
	_scale = scale;
	_stretch = stretch;

	update(_text, _justify);
}

void Text::destroy()
{
	if (_font != nullptr)
		_font->destroyText(this);
}

float Text::getScreenWidth() const
{
	return (_stretch * _scale * _font->getWidth(_text) / 2);
}

float Text::getScreenHeight() const
{
	return (_scale / 2);
}

Rect Text::getScreenRect() const
{
	Rect rect;
	rect.size.width = getScreenWidth();
	rect.size.height = getScreenHeight();
	rect.corner = MigUtil::cameraPlaneToScreenPercent(_pt);
	rect.corner.y -= rect.size.height;

	if (_justify == JUSTIFY_CENTER)
		rect.corner.x -= rect.size.width / 2;
	else if (_justify == JUSTIFY_RIGHT)
		rect.corner.x -= rect.size.width;

	return rect;
}

void Text::draw(const Color& col) const
{
	draw(col.r, col.g, col.b, col.a);
}

void Text::draw(const Color& col, const Matrix& worldMatrix) const
{
	draw(col.r, col.g, col.b, col.a, worldMatrix);
}

void Text::draw(float r, float g, float b, float a) const
{
	if (_text.length() > 0)
	{
		_font->draw(_text, r, g, b, a, _mat);
	}
}

void Text::draw(float r, float g, float b, float a, const Matrix& worldMatrix) const
{
	if (_text.length() > 0)
	{
		static Matrix localMat;
		localMat.copy(_mat);
		localMat.multiply(worldMatrix);

		_font->draw(_text, r, g, b, a, localMat);
	}
}
