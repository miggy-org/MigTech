#include "pch.h"
#include "MigUtil.h"
#include "RenderBase.h"

using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// RenderPass

RenderPass::RenderPass(RenderPassType type) : _type(type), _target(nullptr), _clearColor(colBlack, 0)
{
	_config = 0;
}

RenderPass::~RenderPass()
{
	// this will unload and destroy the render target image
	if (!_name.empty() && MigUtil::theRend != nullptr)
		MigUtil::theRend->unloadImage(_name);
}

// initializes for a render target pass
bool RenderPass::init(const std::string& name, IMG_FORMAT fmtHint, int width, int height, int depthBitsHint)
{
	if (name.empty() || fmtHint == IMG_FORMAT_NONE || width <= 0 || height <= 0)
	{
		LOGWARN("(RenderPass::init) Invalid args");
		return false;
	}

	_name = name;
	_fmtHint = fmtHint;
	_width = width;
	_height = height;
	_depthBitsHint = depthBitsHint;
	_config |= (USE_RENDER_TARGET | USE_CLEAR_COLOR);
	return true;
}

// initializes for a render to a view port pass
bool RenderPass::init(const Rect& viewPort)
{
	_name.clear();
	_viewPort = viewPort;
	_config |= (USE_VIEW_PORT | USE_CLEAR_COLOR | USE_CLEAR_DEPTH);
	return !viewPort.IsEmpty();
}

// simple init
bool RenderPass::init()
{
	_name.clear();
	_config = 0;
	return true;
}

void RenderPass::createGraphics()
{
	if (_name.length() > 0 && _width > 0 && _height > 0)
	{
		_target = MigUtil::theRend->createRenderTarget(_name, _fmtHint, _width, _height, _depthBitsHint);
		if (_target == nullptr)
		{
			// just a warning, no exception
			LOGWARN("(RenderPass::init) Renderer could not create render target");
		}
	}
}

void RenderPass::destroyGraphics()
{
	if (MigUtil::theRend != nullptr && _name.length() > 0)
		MigUtil::theRend->unloadImage(_name);
	_target = nullptr;
}

bool RenderPass::isValid() const
{
	if (_type == RENDER_PASS_NONE)
		return false;

	if (_config & USE_RENDER_TARGET)
		return (_target != nullptr && (_target->getCaps() & IMAGE_CAPS_RENDER_TARGET));
	else if (_config & USE_VIEW_PORT)
		return !_viewPort.IsEmpty();

	return true;
}

bool RenderPass::preRender()
{
	MigUtil::theRend->setProjectionMatrix(_proj);
	MigUtil::theRend->setViewMatrix(_view);
	return true;
}

void RenderPass::postRender()
{
}
