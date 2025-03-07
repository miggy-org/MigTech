#include "pch.h"
#include "PowerUp.h"
#include "GameScripts.h"
#include "../core/MigUtil.h"
#include "../core/Timer.h"

using namespace MigTech;
using namespace Cuboingo;

///////////////////////////////////////////////////////////////////////////
// PowerUp

static const float POWERUP_ICON_ALPHA = 1;

PowerUp::PowerUp() : _type(POWERUPTYPE_NONE), _freeMisses(0), _invStrength(0), _scale(0)
{
}

PowerUp::PowerUp(PowerUpType type) : _type(type), _freeMisses(0), _invStrength(0), _scale(0)
{
	loadClip(_type);
}

// initializes the power-up icon for drawing
void PowerUp::initIcon(const Vector3& ptCenter, const Size& sizeIcon)
{
	_ptCenter = ptCenter;
	_sizeIcon = sizeIcon;
	_scale = 0;
	_color = colWhite;
}

// starts the icon animation after being assigned to a display-able type
void PowerUp::startIconAnim()
{
	loadClip(_type);

	if (_type != POWERUPTYPE_NONE)
	{
		_drawType = _type;

		float fParams[] = { 0.0f, 0.35f, 0.6f, 0.8f, 0.95f, 1.05f, 1.12f, 1.15f, 1.15f, 1.1f, 1.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, 1, 500, fParams, ARRAYSIZE(fParams));
		_idScaleAnim = MigUtil::theAnimList->addItem(animItem);
		_scale = 0;

		// clear the color animation if necessary
		_idColorAnim.clearAnim();
		_color = colWhite;
	}
}

// starts the icon animation when the power-up has been activated
void PowerUp::startIconHitAnim()
{
	if (_scale > 0 && _type != POWERUPTYPE_NONE)
	{
		_idScaleAnim.clearAnim();
		_idCycleAnim.clearAnim();

		float fParams[] = { 1.0f, 1.5f, 1.75f, 1.90f, 2.0f, 1.90f, 1.75f, 1.5f, 1.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, 1, 500, fParams, ARRAYSIZE(fParams));
		_idScaleAnim = MigUtil::theAnimList->addItem(animItem);

		float fParamsC[] = { 0.0f, 0.5f, 0.8f, 0.9f, 1.0f };
		AnimItem animItemC(this);
		animItemC.configParametricAnim(0, 1, 1000, fParamsC, ARRAYSIZE(fParamsC));
		_idColorAnim = MigUtil::theAnimList->addItem(animItemC);
	}
}

// if the icon is already being animated and the type changes, this will update the icon
void PowerUp::updateIconAnim()
{
	// if the scale is non-zero then the icon is displayed and may need an animation update
	if (_scale > 0)
	{
		if (_type == POWERUPTYPE_NONE)
		{
			_idScaleAnim.clearAnim();
			_idCycleAnim.clearAnim();

			float fParams[] = { 1.0f, 1.1f, 1.15f, 1.15f, 1.12f, 1.05f, 0.95f, 0.8f, 0.6f, 0.35f, 0.0f };
			AnimItem animItem(this);
			animItem.configParametricAnim(0, 1, 500, fParams, ARRAYSIZE(fParams));
			_idScaleAnim = MigUtil::theAnimList->addItem(animItem);
		}
		else
			_drawType = _type;
	}
}

static float getIconFrameIndex(PowerUpType type)
{
	return (type == POWERUPTYPE_MULTIPLIER ? 1.0f : 0.0f);
}

// draws the icon
void PowerUp::drawIcon() const
{
	if (_scale > 0 && _drawType != POWERUPTYPE_NONE)
	{
		static Matrix mat;
		mat.identity();
		mat.scale(_scale*_sizeIcon.width, _scale*_sizeIcon.height, 1);
		mat.translate(_ptCenter);
		_mcIcons.jumpToFrame(getIconFrameIndex(_drawType));
		_mcIcons.setColor(_color);
		_mcIcons.draw(mat, false, _scale*POWERUP_ICON_ALPHA);

		//mat.identity();
		//mat.scale(1.2f*_sizeIcon.width, 1.2f*_sizeIcon.width, 1);
		//mat.rotateZ(Timer::gameTimeMillis()*rad45 / 1000.0f);
		//mat.translate(_ptCenter.x, _ptCenter.y, _ptCenter.z);
		//_mcSunRays.draw(mat, false, 0.3f*_scale);
	}
}

// starts the timer that will time the application of the power-up
bool PowerUp::startTimer()
{
	if (_type == POWERUPTYPE_NONE)
		return false;

	long duration = 0;
	const Level lvl = GameScripts::getCurrLevel();
	if (_type == POWERUPTYPE_MULTIPLIER)
		duration = lvl.powerUpMultDur;
	else if (_type == POWERUPTYPE_INVULNERABLE)
	{
		duration = lvl.powerUpInvDur;
		_freeMisses = lvl.powerUpInvMax;
	}

	_invStrength = 1;
	if (duration > 0)
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(1, 0, duration, AnimItem::ANIM_TYPE_LINEAR);
		_idTimer = MigUtil::theAnimList->addItem(animItem);
	}
	return (duration > 0 || _freeMisses > 0);
}

// aborts any display or application timer
void PowerUp::clear()
{
	_type = _drawType = POWERUPTYPE_NONE;
	_idTimer.clearAnim();
	_invStrength = 0;
}

// gets the score multiplier for this power-up
int PowerUp::getScoreMultiplier() const
{
	if (_type != POWERUPTYPE_MULTIPLIER)
		return 1;
	return (GameScripts::getCurrLevel().powerUpMult);
}

// gets the remaining strength of the invulerability power-up
float PowerUp::getInvStrength() const
{
	if (_type != POWERUPTYPE_INVULNERABLE)
		return 0;
	return _invStrength;
}

// applies a miss against the invulnerability power-up
void PowerUp::applyMiss()
{
	if (_freeMisses > 0)
	{
		_freeMisses--;
		if (_freeMisses == 0)
			clear();
		else if (!_idTimer.isActive())
			_invStrength = ((_freeMisses - 1) / (float) (GameScripts::getCurrLevel().powerUpInvMax - 1));
	}
}

bool PowerUp::doFrame(int id, float newVal, void* optData)
{
	if (_idTimer == id)
	{
		_invStrength = newVal;
	}
	else if (_idScaleAnim == id)
	{
		_scale = newVal;
	}
	else if (_idCycleAnim == id)
	{
		_scale = 1 - 0.1f*(float)sin(newVal);
	}
	else if (_idColorAnim == id)
	{
		_color = MigUtil::blendColors(Color(10, 10, 10, 1), colWhite, newVal);
	}
	return true;
}

void PowerUp::animComplete(int id, void* optData)
{
	if (_idTimer == id)
	{
		_idTimer = 0;
		if (_freeMisses == 0)
			clear();
	}
	else if (_idScaleAnim == id)
	{
		_idScaleAnim = 0;

		if (_type != POWERUPTYPE_NONE)
		{
			// start a cycle animation
			AnimItem animItem(this);
			animItem.configSimpleAnim(0, MigTech::rad180, 1000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
			_idCycleAnim = MigUtil::theAnimList->addItem(animItem);
		}
		else
		{
			// we no longer need to draw this icon
			_drawType = POWERUPTYPE_NONE;
			_scale = 0;
		}
	}
	else if (_idColorAnim == id)
	{
		_idColorAnim = 0;

		// clear the icon
		*this = POWERUPTYPE_NONE;
	}
}

///////////////////////////////////////////////////////////////////////////
// static functions for icon drawing

MovieClip PowerUp::_mcIcons;
//MovieClip PowerUp::_mcSunRays;

void PowerUp::createGraphics()
{
	_mcIcons.createGraphics();
	//_mcSunRays.createGraphics();
}

void PowerUp::destroyGraphics()
{
	_mcIcons.destroyGraphics();
	//_mcSunRays.destroyGraphics();
}

bool PowerUp::loadClip(PowerUpType type)
{
	if (type != POWERUPTYPE_NONE)
	{
		if (!_mcIcons.isInit())
		{
			_mcIcons.init("powerups.png", 1, 2, 1, 1, false);
			_mcIcons.create();
			_mcIcons.createGraphics();
		}

		//if (!_mcSunRays.isInit())
		//{
		//	_mcSunRays.init("sun.png", 1, 1);
		//	_mcSunRays.create();
		//	_mcSunRays.createGraphics();
		//}
	}

	return true;
}
