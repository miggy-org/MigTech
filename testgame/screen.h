#pragma once

#include "../core/MigInclude.h"
#include "../core/ScreenBase.h"
#include "../core/MigUtil.h"
#include "../core/Font.h"
#include "../core/MovieClip.h"
#include "../core/Controls.h"
#include "cube.h"
#include "texture.h"

using namespace MigTech;

namespace TestGame
{
	class TestScreen : public MigTech::ScreenBase
	{
	public:
		TestScreen();

		virtual ScreenBase* getNextScreen();

		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();
		virtual void suspend();
		virtual void resume();
		virtual void destroyGraphics();
		virtual void destroy();

		virtual bool pointerPressed(float x, float y);
		virtual void pointerReleased(float x, float y);
		virtual void pointerMoved(float x, float y, bool isInContact);
		virtual void keyDown(VIRTUAL_KEY key);
		virtual void keyUp(VIRTUAL_KEY key);

		virtual bool update();
		virtual bool render();

	protected:
		virtual void onTap(float x, float y);
		virtual void onClick(int id, ControlBase* sender);
		virtual void onSlide(int id, ControlBase* sender, float val);

	private:
		Matrix m_viewMatrix;
		Matrix m_projMatrix;

		Text* m_btmText;

		Cube* m_cubeObj;
		Texture* m_txtObj;

		MigTech::SoundEffect* m_sound;

		float m_xContact;
		float m_yContact;
		bool m_dragCube;
	};
}
