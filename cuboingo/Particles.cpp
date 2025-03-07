#include "pch.h"
#include "CubeUtil.h"
#include "Particles.h"
#include "../core/MigUtil.h"
#include "../core/Timer.h"

using namespace MigTech;
using namespace Cuboingo;

static const int SPARK_MIN_COUNT = 10;        // minimum number of sparks to display
static const int SPARK_MAX_COUNT = 20;        // maximum number of sparks to display
static const int SPARK_DURATION = 500;        // approximate duration of a spark
static const int SPARK_FLICKER_DELAY = 50;    // period of the flicker effect
static const int SPARK_GRAVITY = 2;           // gravity (world coordinates per second^2)

static const float rotCamera2 = 30;

///////////////////////////////////////////////////////////////////////////
// Spark

Spark::Spark()
{
	_callback = nullptr;
	_startTime = 0;
	_flickerScale = 0;
	_lastFlickerUpdate = 0;
}

void Spark::init(const Vector3& pos, const Vector3& vel, const Color& col, long dur, ISparkCallback* callback)
{
	_origPos = _currPos = pos;
	_vel = vel;
	_color = Color(col.r, col.g, col.b, 1);
	_callback = callback;

	// create an animation for the movement
	float tParams[] = { 0.0f, 0.6f, 0.8f, 0.9f, 1.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, 1, dur, tParams, ARRAYSIZE(tParams));
	_idPosAnim = MigUtil::theAnimList->addItem(animItem);

	// create an animation for the alpha channel
	float aParams[] = { 1.0f, 0.9f, 0.8f, 0.6f, 0.0f };
	AnimItem animItemA(this);
	animItemA.configParametricAnim(0, 1, dur, aParams, ARRAYSIZE(aParams));
	_idAlphaAnim = MigUtil::theAnimList->addItem(animItemA);

	// note start time for gravity effect
	_startTime = Timer::gameTimeMillis();
}

void Spark::draw(MovieClip& mc) const
{
	mc.setPos(_currPos);
	mc.setColor(MigUtil::pickRandom(10) == 0 ? colWhite : _color);
	mc.draw();
}

bool Spark::doFrame(int id, float newVal, void* optData)
{
	if (_idPosAnim == id)
	{
		Vector3 delta = _vel * newVal;
		float fallTime = (Timer::gameTimeMillis() - _startTime) / 1000.0f;
		_currPos = Vector3(
			_origPos.x + delta.x,
			_origPos.y + delta.y - (fallTime * fallTime * SPARK_GRAVITY),   // gravity
			_origPos.z + delta.z);
	}
	else if (_idAlphaAnim == id)
	{
		long gameTime = Timer::gameTimeMillis();
		if (gameTime > _lastFlickerUpdate + SPARK_FLICKER_DELAY)
		{
			_lastFlickerUpdate = gameTime;
			_flickerScale = MigUtil::pickFloat();
		}
		_color.a = newVal * _flickerScale;
	}
	return true;
}

void Spark::animComplete(int id, void* optData)
{
	if (_idPosAnim == id)
	{
		_idPosAnim = 0;
		if (_callback != nullptr)
			_callback->sparkComplete(this);
	}
	else if (_idAlphaAnim == id)
		_idAlphaAnim = 0;
}

///////////////////////////////////////////////////////////////////////////
// SparkList

SparkList::SparkList() : _showParticles(true)
{
}

SparkList::~SparkList()
{
}

void SparkList::init()
{
	if (_showParticles)
	{
		const float DIAMETER = 0.2f;
		_mcSpark.init("spark.png", DIAMETER, DIAMETER);
		_mcSpark.setRot(MigUtil::convertToRadians(-rotCamera2), rad45, 0);
	}
}

void SparkList::createGraphics()
{
	if (_showParticles)
		_mcSpark.createGraphics();
}

void SparkList::destroyGraphics()
{
	if (_showParticles)
		_mcSpark.destroyGraphics();
}

static Vector3 orientToVector(AxisOrient orient)
{
	switch (orient)
	{
	case AXISORIENT_X: return Vector3(1, 0, 0);
	case AXISORIENT_Y: return Vector3(0, 1, 0);
	case AXISORIENT_Z: return Vector3(0, 0, 1);
	default: break;
	}
	return Vector3(0, 0, 0);
}

static Vector3 orientToPerturbVector(AxisOrient orient)
{
	switch (orient)
	{
	case AXISORIENT_X: return Vector3(0, 1, 1);
	case AXISORIENT_Y: return Vector3(1, 0, 1);
	case AXISORIENT_Z: return Vector3(1, 1, 0);
	default: break;
	}
	return Vector3(0, 0, 0);
}

void SparkList::sideComplete(AxisOrient orient, const Color& col)
{
	if (_showParticles)
	{
		if (orient == AXISORIENT_NONE)
			throw std::invalid_argument("(SparkList::sideComplete) Axis orientation invalid");
		LOGINFO("(SparkList::sideComplete) Creating particles on axis %s", CubeUtil::axisToString(orient));

		// create a seed vector for the sparks
		Vector3 seedVector = orientToVector(orient);

		int nParticles = SPARK_MIN_COUNT + (MigUtil::pickRandom(SPARK_MAX_COUNT - SPARK_MIN_COUNT + 1));
		for (int i = 0; i < nParticles; i++)
		{
			Vector3 lVector(seedVector);

			// perturb the seed vector to randomize the position and velocities a bit
			Vector3 perturbVector = orientToPerturbVector(orient);
			perturbVector.x *= ((float)MigUtil::pickFloat() * 2 - 1);
			perturbVector.y *= ((float)MigUtil::pickFloat() * 2 - 1);
			perturbVector.z *= ((float)MigUtil::pickFloat() * 2 - 1);
			lVector += perturbVector;

			// randomize the velocity
			float velocityScale = 0.2f + 0.5f * MigUtil::pickFloat();

			// compute initial position and velocity vectors
			Vector3 position(lVector);
			position *= (defCubeRadius * 1.5f);
			Vector3 velocity(lVector);
			velocity *= (defCubeRadius * velocityScale);

			// randomize the spark duration
			long duration = (long)(SPARK_DURATION * (0.5 + MigUtil::pickFloat()));

			// add a new spark
			Spark* newSpark = new Spark();
			newSpark->init(position, velocity, col, duration, this);
			_sparkList.push_back(newSpark);
		}
	}
}

void SparkList::draw()
{
	if (_showParticles)
	{
		_mcSpark.startRenderSet();

		std::list<Spark*>::const_iterator iter = _sparkList.begin();
		while (iter != _sparkList.end())
		{
			(*iter)->draw(_mcSpark);
			iter++;
		}

		_mcSpark.stopRenderSet();
	}
}

void SparkList::sparkComplete(Spark* spark)
{
	if (spark != nullptr)
	{
		_sparkList.remove(spark);
		delete spark;
	}
}
