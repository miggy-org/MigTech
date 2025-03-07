#pragma once

#include "../core/MigInclude.h"
#include "CubeConst.h"

using namespace MigTech;

namespace Cuboingo
{
	class PowerUp : public IAnimTarget
	{
	public:
		PowerUp();
		PowerUp(PowerUpType type);

		// these are for animating and drawing the power-up icon
		void initIcon(const Vector3& ptCenter, const Size& sizeIcon);
		void startIconAnim();
		void startIconHitAnim();
		void updateIconAnim();
		void drawIcon() const;

		// these are for when a power-up is applied
		bool startTimer();
		void clear();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		PowerUpType getType() const { return _type; }
		int getScoreMultiplier() const;
		float getInvStrength() const;
		void applyMiss();

		bool operator == (PowerUpType other) const { return _type == other; }
		bool operator != (PowerUpType other) const { return _type != other; }
		void operator = (PowerUpType other) { _type = other; updateIconAnim(); }

	public:
		// static functions for icon drawing
		static void createGraphics();
		static void destroyGraphics();
		static bool loadClip(PowerUpType type);

	protected:
		// the type of power-up
		PowerUpType _type;

		// power-up application timer
		AnimID _idTimer;
		float _invStrength;

		// miss counter
		int _freeMisses;

		// used for drawing the icon only
		PowerUpType _drawType;
		Vector3 _ptCenter;
		Size _sizeIcon;
		float _scale;
		Color _color;
		AnimID _idScaleAnim;
		AnimID _idCycleAnim;
		AnimID _idColorAnim;

		// static movie clips for icon drawing
		static MovieClip _mcIcons;
		static MovieClip _mcSunRays;
	};
}
