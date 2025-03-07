#pragma once

#include "MigBase.h"
#include "Controls.h"
#include "Font.h"

namespace MigTech
{
	// base class for all dialogs
	class DialogBase : public IAnimTarget, public IControlsCallback
	{
	public:
		DialogBase();

		// init
		virtual void init(IControlsCallback* callback, unsigned int flags = 0, Font* font = nullptr);
		virtual void show();

		// create/destroy
		virtual void create();
		virtual void createGraphics();
		virtual void destroyGraphics();
		virtual void destroy();

		// drawing
		virtual void draw() const;
		virtual void drawContent() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IControlsCallback
		virtual void onClick(int id, ControlBase* sender);
		virtual void onSlide(int id, ControlBase* sender, float val);
		virtual ControlBase* allocControl(const char* tag);

		// input events
		bool pointerPressed(float x, float y);
		void pointerReleased(float x, float y);
		void pointerMoved(float x, float y, bool isInContact);
		bool onTap(float x, float y);

		bool isActive() const { return (_idAnim.isActive() || _alpha > 0); }

	protected:
		IControlsCallback* _callback;
		unsigned int _flags;

		Object* _bgFade;
		Color _fadeColor;
		float _alpha;

		AnimID _idAnim;

		Controls _controls;
		Font* _font;
	};

	// simple message box dialog class
	class SimpleDialog : public DialogBase
	{
	public:
		static const int ID_DIALOG_BTN1 = 1001;
		static const int ID_DIALOG_BTN2 = 1002;

	public:
		// simple static dialog
		static void ShowDialog(
			IControlsCallback* callback,
			const std::string& text,
			const std::string& btn1,
			const std::string& btn2,
			unsigned int flags = 0);

	public:
		SimpleDialog();

		// init
		void setText(const std::string& text);
		void setButtonText(const std::string& btn1, const std::string& btn2);
		void setAccent(const Color& accent);

		virtual void createGraphics();
		virtual void destroyGraphics();
		virtual void drawContent() const;

		virtual void onClick(int id, ControlBase* sender);

		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		int _idBtnClicked;
		Size _totalSize;
		Color _accentColor;

		Object* _bgDialog;
	};
}
