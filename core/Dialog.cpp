#include "pch.h"
#include "MigUtil.h"
#include "Dialog.h"

using namespace MigTech;

/////////////////////////////////////////////////////////////////////
// dialog base class

static const float DIALOG_FADE_OVERLAY_ALPHA = 0.9f;
static const int DIALOG_FADE_DURATION = 250;

DialogBase::DialogBase() :
	_callback(nullptr),
	_bgFade(nullptr),
	_controls(this),
	_font(nullptr)
{
}

void DialogBase::init(IControlsCallback* callback, unsigned int flags, Font* font)
{
	if (callback == nullptr)
		throw std::invalid_argument("(DialogBase::init) Callback cannot be null");
	_callback = callback;
	_flags = flags;

	// font can either be provided or global
	_font = (font != nullptr ? font : MigUtil::theFont);
	_fadeColor = Color(MigTech::colBlack, 0);
	_alpha = 0;

	if (_font == nullptr)
		throw std::invalid_argument("(DialogBase::init) No font available");
}

void DialogBase::show()
{
	if (_callback == nullptr)
		throw std::runtime_error("(DialogBase::show) Must call init() first");

	// start the intro animation
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, DIALOG_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idAnim = MigUtil::theAnimList->addItem(animItem);

	// call create routines
	create();
	createGraphics();
}

void DialogBase::create()
{
	_controls.create();
}

void DialogBase::createGraphics()
{
	// create the fade object and assign the shaders
	_bgFade = MigUtil::theRend->createObject();
	_bgFade->addShaderSet(MIGTECH_VSHADER_POS_NO_TRANSFORM, MIGTECH_PSHADER_COLOR);

	// load mesh vertices (position only)
	VertexPosition txtVertices[4];
	txtVertices[0] = VertexPosition(Vector3(-1, -1, 0));
	txtVertices[1] = VertexPosition(Vector3(-1, 1, 0));
	txtVertices[2] = VertexPosition(Vector3(1, -1, 0));
	txtVertices[3] = VertexPosition(Vector3(1, 1, 0));
	_bgFade->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION);

	// load mesh indices
	const unsigned short txtIndices[] =
	{
		0, 2, 1,
		1, 2, 3,
	};
	_bgFade->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	_bgFade->setCulling(FACE_CULLING_BACK);

	_controls.createGraphics();
}

void DialogBase::destroyGraphics()
{
	_controls.destroyGraphics();

	if (_bgFade != nullptr)
		MigUtil::theRend->deleteObject(_bgFade);
	_bgFade = nullptr;
}

void DialogBase::destroy()
{
	destroyGraphics();

	_controls.removeAllControls();
	_controls.destroy();
}

void DialogBase::draw() const
{
	if (_alpha > 0)
	{
		if (_bgFade)
		{
			MigUtil::theRend->setObjectColor(_fadeColor);
			MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
			MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);
			_bgFade->render();
		}

		drawContent();
	}
}

void DialogBase::drawContent() const
{
	_controls.draw(_alpha);
}

bool DialogBase::pointerPressed(float x, float y)
{
	return _controls.pointerPressed(x, y);
}

void DialogBase::pointerReleased(float x, float y)
{
	_controls.pointerReleased(x, y);
	_controls.onTap(x, y);
}

void DialogBase::pointerMoved(float x, float y, bool isInContact)
{
	_controls.pointerMoved(x, y, isInContact);
}

bool DialogBase::onTap(float x, float y)
{
	return _controls.onTap(x, y);
}

bool DialogBase::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
	{
		// this controls the fade screen opacity (0=no fading, 1=fade is complete)
		_alpha = newVal;
		_fadeColor.a = DIALOG_FADE_OVERLAY_ALPHA*newVal;
	}

	return true;
}

void DialogBase::animComplete(int id, void* optData)
{
	if (_idAnim == id)
		_idAnim = 0;
}

void DialogBase::onClick(int id, ControlBase* sender)
{
}

void DialogBase::onSlide(int id, ControlBase* sender, float val)
{
}

ControlBase* DialogBase::allocControl(const char* tag)
{
	return nullptr;
}

/////////////////////////////////////////////////////////////////////
// simple static dialog

static const float SIMPLE_TEXT_HEIGHT = 0.05f;
static const float ACCENT_OVERLAY_ALPHA = 0.5f;

// utility for a simple dialog
void SimpleDialog::ShowDialog(
	IControlsCallback* callback,
	const std::string& text,
	const std::string& btn1,
	const std::string& btn2,
	unsigned int flags)
{
	SimpleDialog* dlg = new SimpleDialog();
	dlg->init(callback, flags);
	dlg->setText(text);
	dlg->setButtonText(btn1, btn2);
	dlg->setAccent(Color(0.5f, 0.7f, 0.9f));
	dlg->show();

	MigUtil::theDialog = dlg;
}

SimpleDialog::SimpleDialog() : DialogBase(),
	_bgDialog(nullptr),
	_idBtnClicked(0)
{
}

void SimpleDialog::setText(const std::string& text)
{
	_totalSize.width = 1.2f*_font->getWidth(text)*SIMPLE_TEXT_HEIGHT;
	_totalSize.height = 1.5f*_font->getHeight(text)*SIMPLE_TEXT_HEIGHT;

	TextButton* btn = new TextButton(0, false);
	btn->init(_font, MigUtil::getString(text, text), 0.5f, 0.45f, SIMPLE_TEXT_HEIGHT, 1.0f, JUSTIFY_CENTER);
	_controls.addControl(btn);
}

void SimpleDialog::setButtonText(const std::string& btn1, const std::string& btn2)
{
	if (btn1.empty())
		throw std::invalid_argument("(SimpleDialog::setButtonText) At least one button is needed");
	const std::string btnText1 = MigUtil::getString(btn1, btn1);
	const std::string btnText2 = MigUtil::getString(btn2, btn2);

	float wt = _font->getWidth(btnText1)*SIMPLE_TEXT_HEIGHT;
	if (!btnText2.empty())
	{
		float w2 = _font->getWidth(btnText2);
		wt += (w2 + _font->getWidth("     "))*SIMPLE_TEXT_HEIGHT;
	}
	if (1.2f*wt > _totalSize.width)
		_totalSize.width = 1.2f*wt;
	_totalSize.height = 0.4f;

	TextButton* btn;
	if (btnText2.empty())
	{
		btn = new TextButton(ID_DIALOG_BTN1);
		btn->init(_font, btnText1, 0.5f, 0.65f, SIMPLE_TEXT_HEIGHT, 1.0f, JUSTIFY_LEFT);
		_controls.addControl(btn);
	}
	else
	{
		btn = new TextButton(ID_DIALOG_BTN1);
		btn->init(_font, btnText1, 0.5f - wt / 2.0f, 0.65f, SIMPLE_TEXT_HEIGHT, 1.0f, JUSTIFY_LEFT);
		_controls.addControl(btn);
		btn = new TextButton(ID_DIALOG_BTN2);
		btn->init(_font, btnText2, 0.5f + wt / 2.0f, 0.65f, SIMPLE_TEXT_HEIGHT, 1.0f, JUSTIFY_RIGHT);
		_controls.addControl(btn);
	}
}

void SimpleDialog::setAccent(const Color& accent)
{
	_accentColor = Color(accent, ACCENT_OVERLAY_ALPHA);
}

void SimpleDialog::createGraphics()
{
	DialogBase::createGraphics();

	// create the background object and assign the shaders
	_bgDialog = MigUtil::theRend->createObject();
	_bgDialog->addShaderSet(MIGTECH_VSHADER_POS_NO_TRANSFORM, MIGTECH_PSHADER_COLOR);

	// load mesh vertices (position only, note that the totalSize is 2D screen coords)
	float xoff = _totalSize.width;
	float yoff = _totalSize.height;
	VertexPosition txtVertices[4];
	txtVertices[0] = VertexPosition(Vector3(-xoff, -yoff, 0));
	txtVertices[1] = VertexPosition(Vector3(-xoff, yoff, 0));
	txtVertices[2] = VertexPosition(Vector3(xoff, -yoff, 0));
	txtVertices[3] = VertexPosition(Vector3(xoff, yoff, 0));
	_bgDialog->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION);

	// load mesh indices
	const unsigned short txtIndices[] =
	{
		0, 2, 1,
		1, 2, 3,
	};
	_bgDialog->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	_bgDialog->setCulling(FACE_CULLING_BACK);
}

void SimpleDialog::destroyGraphics()
{
	if (_bgDialog != nullptr)
		MigUtil::theRend->deleteObject(_bgDialog);
	_bgDialog = nullptr;

	DialogBase::destroyGraphics();
}

void SimpleDialog::drawContent() const
{
	if (_bgDialog)
	{
		MigUtil::theRend->setObjectColor(_accentColor);
		_bgDialog->render();
	}
	DialogBase::drawContent();
}

void SimpleDialog::onClick(int id, ControlBase* sender)
{
	_idBtnClicked = id;

	// any button will dismiss the dialog
	AnimItem animItem(this);
	animItem.configSimpleAnim(1, 0, DIALOG_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idAnim = MigUtil::theAnimList->addItem(animItem);
}

bool SimpleDialog::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
		_accentColor.a = ACCENT_OVERLAY_ALPHA*newVal;
	return DialogBase::doFrame(id, newVal, optData);
}

void SimpleDialog::animComplete(int id, void* optData)
{
	if (_idAnim == id && _alpha == 0)
	{
		_callback->onClick(_idBtnClicked, nullptr);
		destroy();
	}

	DialogBase::animComplete(id, optData);
}
