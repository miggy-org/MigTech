#pragma once

#include "MigBase.h"
#include "Object.h"
#include "Font.h"
#include "MovieClip.h"
#include "PersistBase.h"

namespace MigTech
{
	class ControlBase;

	enum CONTROL_TYPE {
		UNKNOWN_CONTROL,
		TEXT_BUTTON_CONTROL,
		PIC_BUTTON_CONTROL,
		PIC_TEXT_BUTTON_CONTROL,
		CHECKBOX_BUTTON_CONTROL,
		SLIDER_BUTTON_CONTROL
	};

	// implement this to receive events from various controls
	class IControlsCallback
	{
	public:
		virtual void onClick(int id, ControlBase* sender) = 0;
		virtual void onSlide(int id, ControlBase* sender, float val) = 0;

		virtual ControlBase* allocControl(const char* tag) = 0;
	};

	// base class for all controls
	class ControlBase
	{
	public:
		ControlBase(int id) { _id = id; _type = UNKNOWN_CONTROL; }
		virtual ~ControlBase() {}

		int getID() { return _id; }
		CONTROL_TYPE getType() { return _type; }

		// init
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font) = 0;

		// create/destroy
		virtual void create(IControlsCallback* callback) = 0;
		virtual void createGraphics() = 0;
		virtual void destroyGraphics() = 0;
		virtual void destroy() = 0;

		// drawing
		virtual void draw(float alpha) const = 0;
		virtual void draw(float alpha, const Matrix& worldMatrix) const = 0;

		// input events
		virtual bool pointerPressed(float x, float y) = 0;
		virtual void pointerReleased(float x, float y) = 0;
		virtual void pointerMoved(float x, float y, bool isInContact) = 0;
		virtual bool onTap(float x, float y) = 0;

	protected:
		int _id;
		CONTROL_TYPE _type;
	};

	// basic button class
	class Button : public ControlBase, public IAnimTarget
	{
	public:
		// special ID for the back button
		static const int ID_BACK_BUTTON = 99999;

	public:
		Button(int id);

		// init
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);

		// create/destroy
		virtual void create(IControlsCallback* callback);
		virtual void createGraphics();
		virtual void destroyGraphics();
		virtual void destroy();

		// drawing
		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

		// input events
		virtual bool pointerPressed(float x, float y);
		virtual void pointerReleased(float x, float y);
		virtual void pointerMoved(float x, float y, bool isInContact);
		virtual bool onTap(float x, float y);

		// animation
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// misc
		virtual Color getDrawColor() const;
		virtual void colorUpdated();
		virtual void inflateHitRects(float radius);

	public:
		//void setCallback(IControlsCallback* callback) { _callback = callback; }
		bool isEnabled() const { return _enabled; }
		void setEnabled(bool enabled) { _enabled = enabled; }
		bool isVisible() const { return _visible; }
		void setVisible(bool visible) { _visible = visible; }
		const Color& getColor() const { return _color; }
		void setColor(const Color& color) { _color = color; colorUpdated(); }
		float getAlpha() const { return _color.a; }
		void setAlpha(float alpha) { _color.a = alpha; colorUpdated(); }
		bool isClickable() const { return _clickable; }
		void setClickable(bool clickable) { _clickable = clickable; }
		bool isAnimEnabled() const { return _animEnabled; }
		void setAnimEnabled(bool animEnabled) { _animEnabled = animEnabled; }
		const Rect& getRect() const { return _rect; }

	protected:
		IControlsCallback* _callback;

		bool _enabled;
		bool _visible;
		bool _clickable;
		Color _color;
		Rect _rect;
		float _inflate;

		bool _animEnabled;
		Matrix _mat;
		Vector3 _offset;
		AnimID _idAnim;
		bool _sendClickOnAnimComplete;
	};

	// text button
	class TextButton : public Button
	{
	public:
		TextButton(int id, bool clickable = true) : Button(id) { _type = TEXT_BUTTON_CONTROL; _text = nullptr; _clickable = clickable; }

		void init(Text* text);
		void init(const Font* font, const std::string& text, float u, float v, float h, float stretch, JUSTIFY justify);
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);
		virtual void destroy();

		void updateText(const std::string& newText, JUSTIFY justification = JUSTIFY_IGNORE);
		void updatePos(float u, float v, float h, JUSTIFY justify);
		const std::string& getText() const { return _text->getText(); }

		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

	protected:
		Text* _text;
	};

	// picture button
	class PicButton : public Button
	{
	public:
		PicButton(int id) : Button(id) { _type = PIC_BUTTON_CONTROL; }

		// dimensions are in screen percentages
		void init(const std::string& idTexture, float u, float v, float w, float h, float rot);
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);
		virtual void colorUpdated();

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

	protected:
		MovieClip _mc;
	};

	// picture button w/ text
	class PicTextButton : public PicButton
	{
	public:
		PicTextButton(int id) : PicButton(id) { _type = PIC_TEXT_BUTTON_CONTROL; _text = nullptr; }

		// dimensions are in screen percentages
		void init(const Font* font, const std::string& text, const std::string& idTexture, float u, float v, float w, float h, float rot, float th);
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);

		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

	protected:
		Text* _text;
	};

	// checkbox button
	class CheckBoxButton : public Button
	{
	public:
		CheckBoxButton(int id) : Button(id) { _type = CHECKBOX_BUTTON_CONTROL; _text = nullptr; }

		void init(const std::string& idTexture, float u, float v, float w, float h);
		void init(const std::string& idTexture, float u, float v, float w, float h, const Font* font, const std::string& text, float th);
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);
		virtual void colorUpdated();

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

		virtual bool onTap(float x, float y);

	public:
		bool isChecked() const { return _checked; }
		void setChecked(bool checked) { _checked = checked; _mc.jumpToFrame(_checked); }
		void invertChecked() { setChecked(!_checked); }

	protected:
		MovieClip _mc;
		Text* _text;
		bool _checked;
	};

	// slider button
	class SliderButton : public Button
	{
	public:
		SliderButton(int id) : Button(id) { _type = SLIDER_BUTTON_CONTROL; }

		void init(const std::string& idTextureBg, const std::string& idTextureSlider,
			float ubg, float vbg, float wbg, float hbg,
			float uleft, float uright, float v, float wslider, float hslider);
		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);
		virtual void colorUpdated();

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void draw(float alpha) const;
		virtual void draw(float alpha, const Matrix& worldMatrix) const;

		virtual bool pointerPressed(float x, float y);
		virtual void pointerReleased(float x, float y);
		virtual void pointerMoved(float x, float y, bool isInContact);

	public:
		float getSliderValue() const { return _sliderValue; }
		void setSliderValue(float newVal);

	protected:
		void updateSliderByScreenPosition(float x);

	protected:
		MovieClip _bgClip, _sliderClip;
		float _uSliderL, _uSliderR, _vSlider;
		float _sliderValue;
		bool _isSliding;
	};

	// controls container class
	class Controls : public MigBase
	{
	public:
		Controls(IControlsCallback* callback) { _callback = callback; _inputLocked = nullptr; }

		void addControl(ControlBase* control);
		void removeAllControls();
		void loadFromXML(tinyxml2::XMLElement* xml, const Font* theFont = nullptr);

		// create/destroy
		void createGraphics();
		void destroyGraphics();

		// drawing
		void draw(float alpha) const;
		void draw(float alpha, const Matrix& worldMatrix) const;

		// input events
		bool pointerPressed(float x, float y);
		void pointerReleased(float x, float y);
		void pointerMoved(float x, float y, bool isInContact);
		bool onTap(float x, float y);

		// get a control
		ControlBase* getControlByID(int id) const;

	protected:
		std::vector<ControlBase*> _controls;
		ControlBase* _inputLocked;
		IControlsCallback* _callback;
	};
}