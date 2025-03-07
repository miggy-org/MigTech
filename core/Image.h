#pragma once

#include "MigDefines.h"

namespace MigTech
{
	// render target caps
	static const int IMAGE_CAPS_NONE          = 0;
	static const int IMAGE_CAPS_RENDER_TARGET = 1;
	static const int IMAGE_CAPS_TOP_DOWN      = 2;
	static const int IMAGE_CAPS_BOTTOM_UP     = 4;

	class Image
	{
	protected:
		Image() { _width = _height = 0; _caps = IMAGE_CAPS_NONE; };
		virtual ~Image() { };

	public:
		int getWidth() const { return _width; }
		int getHeight() const { return _height; }
		unsigned int getCaps() const { return _caps; }

	protected:
		int _width;
		int _height;
		unsigned int _caps;
	};
}