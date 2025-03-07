#include "pch.h"
#include "screen.h"
#include "overlay.h"
#include "../core/Perfmon.h"

using namespace TestGame;
using namespace MigTech;

TestScreen::TestScreen()
	: ScreenBase("TestScreen"),
	m_cubeObj(nullptr), m_txtObj(nullptr),
	m_xContact(-1), m_yContact(-1),
	m_sound(nullptr), m_dragCube(false)
{
}

ScreenBase* TestScreen::getNextScreen()
{
	return new TestScreen();
}

void TestScreen::create()
{
	ScreenBase::create();

	// create the font/text
	m_btmText = MigUtil::theFont->createText();
	m_btmText->init(MigUtil::getString("score") + ": 0", JUSTIFY_CENTER);
	m_btmText->transform(0.25f, 1, 0, -1, 0, -rad90, 0, 0);

	// create the cube/texture objects
	m_cubeObj = new Cube();
	//m_txtObj = new Texture();

	// background image
	//initBackgroundScreen("splash.png");

	// load the music
	//startMusic("splash.mp3");
	if (MigUtil::theAudio != nullptr)
		m_sound = MigUtil::theAudio->loadMedia("match.wav", AudioBase::AUDIO_CHANNEL_SOUND);

	// controls
	//TextButton* btn = new TextButton(1);
	//btn->init(MigUtil::theFont, "OVERLAY", 0.17f, 0.95f, 0.05f, JUSTIFY_CENTER);
	//_controls.addControl(btn);
	//PicButton* btn = new PicButton(2);
	//btn->init("pngtest.png", 0.7f, 0.3f, 0.2f, 0.2f, 0);
	//_controls.addControl(btn);
	//CheckBoxButton* btn = new CheckBoxButton(3);
	//btn->init("checkbox.png", 0.05f, 0.95f, 0.05f, 0.05f, &m_theFont, "CHECK", 0.05f);
	//_controls.addControl(btn);
	//SliderButton* btn = new SliderButton(4);
	//btn->init("slider_bg.png", "slider.png", 0.3f, 0.95f, 0.4f, 0.1f, 0.1f, 0.5f, 0.95f, 0.03f, 0.08f);
	//_controls.addControl(btn);

	// restore translation
	float dx = MigUtil::thePersist->getValue("xtest", (float) 0);
	float dy = MigUtil::thePersist->getValue("ytest", (float) 0);
	if (m_cubeObj != nullptr)
		m_cubeObj->Translate(2 * dx, -2 * dy);

	PerfMon::init(MigUtil::theFont);
	PerfMon::showFPS(true);
}

void TestScreen::createGraphics()
{
	ScreenBase::createGraphics();

	if (m_cubeObj != nullptr)
		m_cubeObj->CreateDeviceDependentResources();
	if (m_txtObj != nullptr)
		m_txtObj->CreateDeviceDependentResources();
}

void TestScreen::windowSizeChanged()
{
	ScreenBase::windowSizeChanged();

	// TODO: Replace this with the size-dependent initialization of your app's content.
	MigTech::Size outputSize = MigUtil::theRend->getOutputSize();
	float aspectRatio = outputSize.width / outputSize.height;
	float fovAngleY = 70.0f * MigTech::PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.
	//MigUtil::theRend->setProjectionMatrix(fovAngleY, aspectRatio, 0.01f, 100.0f, true);
	m_projMatrix.loadPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const MigTech::Vector3 eyev(0.0f, 0.7f, 1.5f);
	static const MigTech::Vector3 atv(0.0f, -0.1f, 0.0f);
	static const MigTech::Vector3 upv(0.0f, 1.0f, 0.0f);
	//MigUtil::theRend->setViewMatrix(eyev, atv, upv);
	m_viewMatrix.loadLookAtRH(eyev, atv, upv);
}

void TestScreen::suspend()
{
	ScreenBase::suspend();

	MigUtil::thePersist->commit();
}

void TestScreen::resume()
{
	ScreenBase::resume();
}

void TestScreen::destroyGraphics()
{
	if (m_cubeObj != nullptr)
		m_cubeObj->ReleaseDeviceDependentResources();
	if (m_txtObj != nullptr)
		m_txtObj->ReleaseDeviceDependentResources();

	ScreenBase::destroyGraphics();
}

void TestScreen::destroy()
{
	if (m_sound != nullptr && MigUtil::theAudio != nullptr)
		MigUtil::theAudio->deleteMedia(m_sound);
	m_sound = nullptr;

	if (m_txtObj != nullptr)
		delete m_txtObj;
	m_txtObj = nullptr;
	if (m_cubeObj != nullptr)
		delete m_cubeObj;
	m_cubeObj = nullptr;

	if (m_btmText != nullptr)
		m_btmText->destroy();
	m_btmText = nullptr;

	ScreenBase::destroy();
}

bool TestScreen::pointerPressed(float x, float y)
{
	if (!ScreenBase::pointerPressed(x, y))
	{
		m_dragCube = true;
		m_xContact = x;
		m_yContact = y;
		//if (m_cubeObj != nullptr)
		//	m_cubeObj->Translate(0, 0);
	}

	return true;
}

void TestScreen::pointerReleased(float x, float y)
{
	ScreenBase::pointerReleased(x, y);

	if (m_dragCube)
	{
		MigUtil::thePersist->putValue("xtest", x - m_xContact);
		MigUtil::thePersist->putValue("ytest", y - m_yContact);
		if (m_sound != nullptr)
			m_sound->playSound(false);
		m_dragCube = false;
	}

	m_xContact = -1;
	m_yContact = -1;
}

void TestScreen::pointerMoved(float x, float y, bool isInContact)
{
	ScreenBase::pointerMoved(x, y, isInContact);

	if (isInContact && m_dragCube)
	{
		float dx = x - m_xContact;
		float dy = y - m_yContact;
		if (m_cubeObj != nullptr)
			m_cubeObj->Translate(2 * dx, -2 * dy);
	}
}

void TestScreen::keyDown(VIRTUAL_KEY key)
{
	ScreenBase::keyDown(key);

	if (m_cubeObj != nullptr)
	{
		if (key == NUMPAD4 || key == LEFT)
			m_cubeObj->RotateDir(-1);
		else if (key == NUMPAD6 || key == RIGHT)
			m_cubeObj->RotateDir(1);
	}
}

void TestScreen::keyUp(VIRTUAL_KEY key)
{
	ScreenBase::keyUp(key);
}

void TestScreen::onTap(float x, float y)
{
	if (!_controls.onTap(x, y) && m_cubeObj != nullptr)
		m_cubeObj->Translate(0, 0);
}

void TestScreen::onClick(int id, ControlBase* sender)
{
	if (id == 1)
	{
		//startNewOverlay(new TestOverlay());
		SimpleDialog::ShowDialog(this,
			"VIEW THE OVERLAY?",
			"OK", "CANCEL");
	}
	else if (id == SimpleDialog::ID_DIALOG_BTN1)
		startNewOverlay(new TestOverlay());
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

void TestScreen::onSlide(int id, ControlBase* sender, float val)
{
	if (m_btmText != nullptr)
	{
		char number[12];
		sprintf(number, "SCORE: %d", (int) (100*val));
		m_btmText->update(number, JUSTIFY_CENTER);
	}
}

#pragma warning(pop)

bool TestScreen::update()
{
	return ScreenBase::update();
}

bool TestScreen::render()
{
	// background screen
	drawBackgroundScreen();

	MigUtil::theRend->setProjectionMatrix(nullptr);
	MigUtil::theRend->setViewMatrix(nullptr);

	MigUtil::theRend->setProjectionMatrix(m_projMatrix);
	MigUtil::theRend->setViewMatrix(m_viewMatrix);
	m_btmText->draw(0.5f, 0.5f, 1, 0.5f);
	if (m_txtObj != nullptr)
		m_txtObj->Render();
	if (m_cubeObj != nullptr)
		m_cubeObj->Render();

	return true;
}
