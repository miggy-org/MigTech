#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/MovieClip.h"
#include "CubeConst.h"

using namespace MigTech;

namespace Cuboingo
{
	class Spark;

	class ISparkCallback
	{
	public:
		virtual void sparkComplete(Spark* spark) = 0;
	};

	class Spark : public IAnimTarget
	{
	public:
		Spark();

		void init(const Vector3& pos, const Vector3& vel, const Color& col, long dur, ISparkCallback* callback);
		void draw(MovieClip& mc) const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		ISparkCallback* _callback;
		Vector3 _origPos, _currPos, _vel;
		Color _color;

		AnimID _idPosAnim, _idAlphaAnim;
		long _startTime;
		float _flickerScale;
		long _lastFlickerUpdate;
	};

	class SparkList : public MigBase, public ISparkCallback
	{
	public:
		SparkList();
		~SparkList();

		void init();

		void createGraphics();
		void destroyGraphics();

		void sideComplete(AxisOrient orient, const Color& col);
		void draw();

		void setShowParticles(bool show) { _showParticles = show; }

		// ISparkCallback
		virtual void sparkComplete(Spark* spark);

	protected:

	protected:
		std::list<Spark*> _sparkList;
		MovieClip _mcSpark;
		bool _showParticles;
	};
}
