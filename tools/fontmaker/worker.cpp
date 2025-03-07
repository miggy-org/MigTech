// worker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../../core/tinyxml/tinyxml2.h"
#include "../../core/libpng/png.h"

using namespace tinyxml2;

struct CharEntry
{
	wchar_t c;
	int left;
	int top;
	int width;

	CharEntry(wchar_t newChar) { c = newChar; left = top = width = 0; }
};

// configuration globals
int _widthImage = 0;
int _heightImage = 0;
int _spacing = 0;
bool _useAscent = false;
std::vector<CharEntry> _charList;

// bitmap globals
HDC _hMemDC = NULL;
HBITMAP _hBmp = NULL;
int _fontMaxWidth = 0;
int _fontHeight = 0;
std::wstring _fontName;

static BYTE* loadFileBuffer(const wchar_t* filePath, int& length)
{
	BYTE* pFile = nullptr;
	length = 0;

	// retrieve the file size
	struct _stat st;
	if (_wstat(filePath, &st) == 0)
	{
		length = st.st_size;

		// allocate a buffer
		pFile = new BYTE[length];
		if (pFile != nullptr)
		{
			FILE* pf = nullptr;
			if (_wfopen_s(&pf, filePath, L"rb") == 0)
			{
				if (fread_s(pFile, length, 1, length, pf) < (size_t)length)
				{
					delete pFile;
					pFile = nullptr;
				}

				fclose(pf);
			}
			else
			{
				delete pFile;
				pFile = nullptr;
			}
		}
	}

	return pFile;
}

static int safeParseInt(const char* str, int def)
{
	if (str != nullptr)
		return atoi(str);
	return def;
}

static bool safeParseBool(const char* str, bool def)
{
	if (str != nullptr)
		return (_stricmp(str, "true") == 0 ? true : false);
	return def;
}

// parses and loads settings from the input script
bool ParseInputXML(const wchar_t* xmlFile)
{
	if (xmlFile != nullptr && xmlFile[0] != 0)
	{
		int length;
		BYTE* data = loadFileBuffer(xmlFile, length);
		if (data != nullptr)
		{
			tinyxml2::XMLDocument* pdoc = new tinyxml2::XMLDocument();
			XMLError err = pdoc->Parse((const char*)data, length);
			if (err == XML_SUCCESS)
			{
				XMLElement* elem = pdoc->FirstChildElement("fontmaker");
				if (elem != nullptr)
				{
					_widthImage = safeParseInt(elem->Attribute("width"), _widthImage);
					_heightImage = safeParseInt(elem->Attribute("height"), _heightImage);
					_spacing = safeParseInt(elem->Attribute("spacing"), _spacing);
					_useAscent = safeParseBool(elem->Attribute("useascent"), _useAscent);

					elem = elem->FirstChildElement("item");
					while (elem != nullptr)
					{
						const char* p = elem->Attribute("char");
						if (p != nullptr)
							_charList.push_back(CharEntry(*p));
						elem = elem->NextSiblingElement("item");
					}
				}
			}

			delete pdoc;
		}
	}

	return (_widthImage > 0 && _heightImage > 0);
}

static int getCharWidth(HDC hDC, wchar_t c, bool isTT)
{
	if (isTT)
	{
		ABC abc;
		if (GetCharABCWidths(hDC, c, c, &abc))
			return abc.abcA + abc.abcB + abc.abcC;
	}
	else
	{
		int width;
		if (GetCharWidth(hDC, c, c, &width))
			return width;
	}

	return 0;
}

static void renderChars(HDC hDC, HFONT hFont)
{
	RECT rect;
	rect.right = 9999;
	rect.bottom = 9999;

	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkColor(hDC, RGB(0, 0, 0));

	TEXTMETRIC tm;
	if (GetTextMetrics(hDC, &tm))
	{
		_fontMaxWidth = 0; // tm.tmMaxCharWidth;
		_fontHeight = (_useAscent ? tm.tmAscent : tm.tmHeight);

		int currX = 0, currY = 0;
		for (int i = 0; i < (int)_charList.size(); i++)
		{
			wchar_t c = _charList[i].c;

			int charWidth = getCharWidth(hDC, c, (tm.tmPitchAndFamily & TMPF_TRUETYPE ? true : false)) + _spacing;
			if (charWidth > 0)
			{
				if (currX + charWidth > _widthImage)
				{
					currX = 0;
					currY += (_fontHeight + _spacing);
					if (currY > _heightImage)
						break;
				}

				rect.left = currX;
				rect.top = currY;
				DrawText(hDC, &c, 1, &rect, DT_LEFT | DT_TOP);

				_charList[i].left = currX;
				_charList[i].top = currY;
				_charList[i].width = charWidth;

				if (charWidth > _fontMaxWidth)
					_fontMaxWidth = charWidth;

				currX += charWidth;
			}
		}
	}
}

// creates a bitmap from the selected font and renders all of the characters
bool CreateBitmapFromSelectedFont(LPLOGFONT lf, HWND hWnd)
{
	if (lf == nullptr)
		return false;

	if (_hMemDC != NULL)
		DeleteDC(_hMemDC);
	if (_hBmp != NULL)
		DeleteObject(_hBmp);

	HDC hDC = GetDC(hWnd);
	_hMemDC = CreateCompatibleDC(hDC);
	_hBmp = CreateCompatibleBitmap(hDC, _widthImage, _heightImage);
	ReleaseDC(hWnd, hDC);

	HFONT hFont = CreateFontIndirect(lf);
	HGDIOBJ hOldFont = SelectObject(_hMemDC, hFont);
	HGDIOBJ hOld = SelectObject(_hMemDC, _hBmp);
	renderChars(_hMemDC, hFont);
	SelectObject(_hMemDC, hOld);
	SelectObject(_hMemDC, hOldFont);
	DeleteObject(hFont);

	_fontName = lf->lfFaceName;
	return true;
}

// resizes the given window to match the loaded bitmap dimensions
void ResizeWindowToMatchBitmap(HWND hWnd)
{
	RECT rc = { 0, 0, _widthImage, _heightImage };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
	SetWindowPos(hWnd, NULL, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOMOVE | SWP_NOZORDER);

	InvalidateRect(hWnd, NULL, TRUE);
}

// paints the bitmap, if it exists
void PaintBitmap(HDC hDC)
{
	if (_hMemDC != NULL && _hBmp != NULL)
	{
		HGDIOBJ hOld = SelectObject(_hMemDC, _hBmp);
		BitBlt(hDC, 0, 0, _widthImage, _heightImage, _hMemDC, 0, 0, SRCCOPY);
		SelectObject(_hMemDC, hOld);
	}
}

static bool SaveFontImage(const wchar_t* name)
{
	if (name == NULL || name[0] == 0 || _hMemDC == NULL || _hBmp == NULL)
		return false;

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = _widthImage;
	bmi.biHeight = _heightImage;
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biCompression = BI_RGB;
	LPBYTE pBits = (LPBYTE)VirtualAlloc(NULL, 4 * _widthImage*_heightImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!GetDIBits(_hMemDC, _hBmp, 0, _heightImage, pBits, (BITMAPINFO *)&bmi, DIB_RGB_COLORS))
		return false;

	int row_stride = 4 * _widthImage;
	for (int y = 0; y < _heightImage; y++)
	{
		for (int x = 0; x < _widthImage; x++)
		{
			int r = pBits[row_stride*y + 4 * x + 0];
			int g = pBits[row_stride*y + 4 * x + 1];
			int b = pBits[row_stride*y + 4 * x + 2];
			pBits[row_stride*y + 4 * x + 3] = (BYTE) ((r + g + b) / 3);
		}
	}

	std::wstring pngName = name;
	if (pngName.find(L".png") == std::wstring::npos)
		pngName += L".png";

	FILE* fp;
	_wfopen_s(&fp, pngName.c_str(), L"wb");
	if (!fp)
		return false;

	png_structp png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
		return false;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, _widthImage, _heightImage,
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_bytepp row_pointers = new png_bytep[_heightImage];
	for (int y = 0; y < _heightImage; y++)
		row_pointers[y] = pBits + (_heightImage - y - 1)*row_stride;
	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	delete[] row_pointers;
	VirtualFree(pBits, 0, MEM_RELEASE);
	return true;
}

static bool SaveFontConfiguration(const wchar_t* name)
{
	if (name == NULL || name[0] == 0)
		return false;

	std::wstring xmlName = name;
	int end = xmlName.find(L".png");
	if (end != std::wstring::npos)
		xmlName.erase(end);
	xmlName += L"_cfg.xml";

	FILE* fp;
	_wfopen_s(&fp, xmlName.c_str(), L"w");
	if (!fp)
		return false;

	fwprintf_s(fp, L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fwprintf_s(fp, L"<Font\n\n");
	fwprintf_s(fp, L"\tName=\"%s\"\n", _fontName.c_str());
	fwprintf_s(fp, L"\tUseAlpha=\"true\"\n");
	fwprintf_s(fp, L"\tAddAlpha=\"false\"\n");
	fwprintf_s(fp, L"\tDropColor=\"true\"\n");
	fwprintf_s(fp, L"\tWidth=\"%d\"\n", _fontMaxWidth);
	fwprintf_s(fp, L"\tHeight=\"%d\"\n", _fontHeight);
	fwprintf_s(fp, L"\tSpacing=\"1\" >\n\n");

	for (int i = 0; i < (int) _charList.size(); i++)
	{
		const CharEntry& entry = _charList[i];
		fwprintf_s(fp, L"\t<Item Char=\"%c\" Left=\"%d\" Top=\"%d\" Width=\"%d\" />\n", entry.c, entry.left, entry.top, entry.width);
	}

	fwprintf_s(fp, L"\n</Font>\n");

	fclose(fp);
	return true;
}

// saves the font bitmap and the configuration to storage
bool SaveFontImageAndConfiguration(const wchar_t* name)
{
	if (!SaveFontImage(name))
		return false;
	return SaveFontConfiguration(name);
}
