#include "pch.h"
#include "MigConst.h"
#include "MovieClip.h"
#include "MigUtil.h"

using namespace MigTech;

const std::string movieClipVertexShader = "mtvs_MovieClip";
const std::string movieClipPixelShader = MIGTECH_PSHADER_TEX;

MovieClip::MovieClip()
	: _screenPoly1(nullptr), _screenPoly2(nullptr), _txtVerts(nullptr),
	_rowCount(0), _colCount(0), _width(0), _height(0),
	_frameCount(0), _blendFrames(false), _playHead(0), _visible(false),
	_rX(0), _rY(0), _rZ(0), _sX(1), _sY(1),
	_callback(nullptr)
{
}

bool MovieClip::init(const std::string& bmpID, int rowCount, int colCount, float mcW, float mcH, bool blendFrames)
{
	if (_screenPoly1 != nullptr && _blendFrames != blendFrames)
		throw std::invalid_argument("(MovieClip::init) Cannot switch blend frames after first init");

	// if init() has already been called, dump the previous image
	if (_screenPoly1 != nullptr && !_bmpResID.empty())
		MigUtil::theRend->unloadImage(_bmpResID);

	_mat.identity();
	_bmpResID = bmpID;
	_colCount = colCount;
	_rowCount = rowCount;
	_width = mcW;
	_height = mcH;
	_frameCount = rowCount * colCount;
	_blendFrames = blendFrames;
	_color = MigTech::colWhite;
	_visible = true;

	// start at the first frame
	_playHead = -1;
	jumpToFrame(0);

	// if init() has already been called, load the new image now
	if (_screenPoly1 != nullptr)
	{
		if (MigUtil::theRend->loadImage(_bmpResID, _bmpResID, LOAD_IMAGE_NONE) != nullptr)
		{
			_screenPoly1->setImage(0, _bmpResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
			if (_screenPoly2 != nullptr)
				_screenPoly2->setImage(0, _bmpResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
		}
	}

	return true;
}

bool MovieClip::init(const std::string& bmpID, float mcW, float mcH)
{
	return init(bmpID, 1, 1, mcW, mcH, false);
}

void MovieClip::createGraphics()
{
	MigUtil::theRend->loadVertexShader(movieClipVertexShader, VDTYPE_POSITION_TEXTURE, SHADER_HINT_MVP);
	//MigUtil::theRend->loadPixelShader(movieClipPixelShader, SHADER_HINT_NONE);

	// load the texture map
	if (!_bmpResID.empty() && MigUtil::theRend->loadImage(_bmpResID, _bmpResID, LOAD_IMAGE_NONE) != nullptr)
	{
		// create the texture objects and assign the shaders
		_screenPoly1 = MigUtil::theRend->createObject();
		if (_blendFrames)
			_screenPoly2 = MigUtil::theRend->createObject();
		_screenPoly1->addShaderSet(movieClipVertexShader, movieClipPixelShader);
		if (_blendFrames)
			_screenPoly2->addShaderSet(movieClipVertexShader, movieClipPixelShader);

		// create vertex array (position set now, uv coords set later)
		_txtVerts = new VertexPositionTexture[4];
		_txtVerts[0].pos = Vector3(-_width / 2, -_height / 2, 0);
		_txtVerts[1].pos = Vector3(-_width / 2,  _height / 2, 0);
		_txtVerts[2].pos = Vector3( _width / 2, -_height / 2, 0);
		_txtVerts[3].pos = Vector3( _width / 2,  _height / 2, 0);
		_txtVerts[0].uv = Vector2(0, 1);
		_txtVerts[1].uv = Vector2(0, 0);
		_txtVerts[2].uv = Vector2(1, 1);
		_txtVerts[3].uv = Vector2(1, 0);
		_screenPoly1->loadVertexBuffer(_txtVerts, 4, MigTech::VDTYPE_POSITION_TEXTURE);
		if (_blendFrames)
			_screenPoly2->loadVertexBuffer(_txtVerts, 4, MigTech::VDTYPE_POSITION_TEXTURE);

		// load mesh indices
		static const unsigned short txtIndices[] =
		{
			0, 2, 1,
			1, 2, 3,
		};
		_screenPoly1->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);
		if (_blendFrames)
			_screenPoly2->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

		// texturing
		_screenPoly1->setImage(0, _bmpResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
		if (_blendFrames)
			_screenPoly2->setImage(0, _bmpResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

		// culling
		_screenPoly1->setCulling(FACE_CULLING_BACK);
		if (_blendFrames)
			_screenPoly2->setCulling(FACE_CULLING_BACK);
	}
}

void MovieClip::destroyGraphics()
{
	if (_txtVerts)
		delete[] _txtVerts;
	_txtVerts = nullptr;
	if (_screenPoly1)
		MigUtil::theRend->deleteObject(_screenPoly1);
	_screenPoly1 = nullptr;
	if (_screenPoly2)
		MigUtil::theRend->deleteObject(_screenPoly2);
	_screenPoly2 = nullptr;
	//MigUtil::theRend->unloadImage(_bmpResID);
}

void MovieClip::drawFrame(bool first, float param, float alpha) const
{
	MigUtil::theRend->setObjectColor(Color(_color, _color.a*param*alpha));

	if (first && _screenPoly1 != nullptr)
		_screenPoly1->render();
	else if (!first && _screenPoly2 != nullptr)
		_screenPoly2->render();
}

void MovieClip::drawFrames(bool depth, float alpha) const
{
	MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);

	// draw the first frame, and maybe the second if frame blending is enabled
	float playHead = (_blendFrames ? _playHead : (int)_playHead);
	float param = 1.0f - (playHead - (int)playHead);
	bool needBothFrames = (_blendFrames && param < 1);

	MigUtil::theRend->setDepthTesting(depth ? DEPTH_TEST_STATE_LESS : DEPTH_TEST_STATE_NONE, (depth && !needBothFrames));
	MigUtil::theRend->setMiscValue(0, (float)((int)_playHead));
	MigUtil::theRend->setMiscValue(1, (float)_rowCount);
	MigUtil::theRend->setMiscValue(2, (float)_colCount);
	drawFrame(true, param, alpha);

	if (needBothFrames)
	{
		MigUtil::theRend->setDepthTesting(depth ? DEPTH_TEST_STATE_LESS : DEPTH_TEST_STATE_NONE, depth);
		MigUtil::theRend->setMiscValue(0, (float)((int)_playHead + 1));
		drawFrame(false, 1.0f - param, alpha);
	}
}

void MovieClip::draw(const Matrix& worldMatrix, bool depth, float alpha) const
{
	if (_visible)
	{
		static Matrix localMat;
		localMat.copy(_mat);
		localMat.multiply(worldMatrix);
		MigUtil::theRend->setModelMatrix(localMat);

		drawFrames(depth, alpha);
	}
}

void MovieClip::draw(float alpha) const
{
	if (_visible)
	{
		MigUtil::theRend->setModelMatrix(_mat);

		drawFrames(false, alpha);
	}
}

bool MovieClip::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
		jumpToFrame(newVal);
	return true;
}

void MovieClip::animComplete(int id, void* optData)
{
	if (_idAnim == id)
	{
		_idAnim = 0;

		// invoke the callback to inform the caller that this clip is done playing
		if (_callback != nullptr)
			_callback->movieComplete(this);
	}
}

// jumps immediately to the given frame, no animation
void MovieClip::jumpToFrame(float frame)
{
	if (frame < 0)
		throw std::invalid_argument("(MovieClip::jumpToFrame) Frame cannot be less than 0");
	if (frame > _frameCount - 1)
		throw std::invalid_argument("(MovieClip::jumpToFrame) Frame cannot exceed frame count");
	//LOGINFO("(MovieClip::jumpToFrame) Jumping to frame %f", frame);
	_playHead = frame;
}

// plays to the given frame from the current play head position
void MovieClip::playToFrame(float frame, long duration, IMovieClipCallback* callback)
{
	if (frame < 0)
		throw std::invalid_argument("(MovieClip::playToFrame) Frame cannot be less than 0");
	if (frame > _frameCount - 1)
		throw std::invalid_argument("(MovieClip::playToFrame) Frame cannot exceed frame count");
	//LOGINFO("(MovieClip::playToFrame) Playing to frame %f", frame);

	AnimItem animItem(this);
	animItem.configSimpleAnim(_playHead, frame, duration, AnimItem::ANIM_TYPE_LINEAR);
	_idAnim = MigUtil::theAnimList->addItem(animItem);

	_callback = callback;
}

void MovieClip::applyTransformations()
{
	_mat.identity();
	if (_sX != 1 || _sY != 1)
		_mat.scale(_sX, _sY, 1);
	if (_rX != 0)
		_mat.rotateX(_rX);
	if (_rY != 0)
		_mat.rotateY(_rY);
	if (_rZ != 0)
		_mat.rotateZ(_rZ);
	_mat.translate(_pos.x, _pos.y, _pos.z);
}

void MovieClip::startRenderSet()
{
	_screenPoly1->startRenderSet();
	if (_blendFrames && _screenPoly2 != nullptr)
		_screenPoly2->startRenderSet();
}

void MovieClip::stopRenderSet()
{
	_screenPoly1->stopRenderSet();
	if (_blendFrames && _screenPoly2 != nullptr)
		_screenPoly2->stopRenderSet();
}
