#pragma once

#include "tinyxml\tinyxml2.h"

#include "MigBase.h"
#include "Object.h"
#include "AnimList.h"

namespace MigTech
{
	class BgBase : public MigBase
	{
	public:
		static BgBase* loadHandlerFromXML(tinyxml2::XMLElement* xml);

	public:
		BgBase();
		BgBase(const std::string& bgResID);
		virtual ~BgBase();

		bool init(const std::string& bgResID);
		bool isVisible() const { return _isVisible; }
		void setVisible(bool vis) { _isVisible = vis; }
		bool isOpaque() const { return _isOpaque; }
		void setOpaque(bool op) { _isOpaque = op; }

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void update();
		virtual void render() const;

	protected:
		std::string _bgResID;
		Object* _screenPoly;
		bool _isVisible;
		bool _isOpaque;
	};

	class CycleBg : public BgBase, public IAnimTarget
	{
	public:
		CycleBg();
		CycleBg(const std::string& image1, const std::string& image2, long duration);

		bool init(const std::string& image1, const std::string& image2, long duration);

		virtual void create();
		virtual void createGraphics();
		virtual void destroyGraphics();
		virtual void destroy();

		virtual void render() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		std::string _bgRes2ID;
		Object* _screenPoly2;
		Color _colOther;
		AnimID _idAnim;
		long _dur;
	};

	class OverlayBg : public BgBase, public IAnimTarget
	{
	public:
		OverlayBg();

		bool initImage(const std::string& image, float uPeriod, float vPeriod);
		bool initOverlay(const std::string& image, float uPeriod, float vPeriod);

		virtual void create();
		virtual void createGraphics();
		virtual void destroyGraphics();
		virtual void destroy();

		virtual void render() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		std::string _bgResOverlay;
		Object* _overlayPoly;
		AnimID _idAnim;

		float _uPeriodBg, _vPeriodBg;
		float _uPeriodOv, _vPeriodOv;
		float _uBgOffset, _vBgOffset;
		float _uOvOffset, _vOvOffset;
	};
}
