///////////////////////////////////////////////////////////////////////////
// platform specific

#include "pch.h"
#include "WinApp.h"
#include "../core/MigInclude.h"
#include "DxRender.h"
#include "XAudio.h"
#include "LocalPersist.h"

#include <ppltasks.h>

using namespace MigTech;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::System::Threading;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
using namespace Windows::Phone::UI::Input;
#endif

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new WinApp();
}

WinApp::WinApp() :
	m_initComplete(false),
	m_windowClosed(false),
	m_windowVisible(true),
	m_timerWatchdog(nullptr),
	m_lastKeyDown(VirtualKey::None)
{
}

// The first method called when the IFrameworkView is being created.
void WinApp::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinApp::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &WinApp::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &WinApp::OnResuming);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	//m_deviceResources = std::unique_ptr<MigTech::DxRender>(new MigTech::DxRender());

	try
	{
		// initialize MigTech
		MigTech::AudioBase* pAudio = new MigTech::XAudioManager();
		MigTech::PersistBase* pDataManager = new MigTech::LocalPersist();
		MigTech::MigGame::initGameEngine(pAudio, pDataManager);
		MigTech::RenderBase* pRtObj = new MigTech::DxRender();
		MigTech::MigGame::initRenderer(pRtObj);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}

// Called when the CoreWindow object is created (or re-created).
void WinApp::SetWindow(CoreWindow^ window)
{
	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinApp::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WinApp::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinApp::OnDisplayContentsInvalidated);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinApp::OnOrientationChanged);

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinApp::OnWindowSizeChanged);

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinApp::OnDpiChanged);

	// Disable all pointer visual feedback for better performance when touching.
	// This is not supported on Windows Phone applications.
	auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerVisualizationSettings->IsContactFeedbackEnabled = false; 
	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
#else
	HardwareButtons::BackPressed +=
		ref new EventHandler<BackPressedEventArgs^>(this, &WinApp::OnBackKey);
#endif

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinApp::OnPointerPressed);
	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinApp::OnPointerReleased);
	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinApp::OnPointerMoved);
	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinApp::OnKeyDown);
	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinApp::OnKeyUp);

	try
	{
		((MigTech::DxRender*) MigTech::MigUtil::theRend)->SetWindow(window);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}

// Initializes scene resources, or loads a previously saved app state.
void WinApp::Load(Platform::String^ entryPoint)
{
	try
	{
		if (m_main == nullptr)
		{
			m_main = std::unique_ptr<MigTech::MigGame>(allocGame());

			Concurrency::create_task([this]()
			{
				m_main->onCreate();
				m_main->onCreateGraphics();
				m_main->onWindowSizeChanged();
				m_initComplete = true;

				// start up the watchdog thread
				int watchdogPeriod = MigUtil::getWatchdogPeriod();
				if (watchdogPeriod > 0)
				{
					LOGINFO("(WinApp::Load) Starting watchdog thread, period is %d seconds", watchdogPeriod);

					// watchdog period
					TimeSpan period;
					period.Duration = watchdogPeriod * 10000000;

					// start a watchdog thread that will make sure the watchdog is being petted
					m_timerWatchdog = ThreadPoolTimer::CreatePeriodicTimer(
						ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
					{
						if (!MigUtil::checkWatchdog())
						{
							MigUtil::fatal("Watchdog triggered, dumping log file");
							MigUtil::dumpLogToFile();

							// log dumped so cancel the watchdog
							m_timerWatchdog->Cancel();
						}
					}), period);
				}
			});
		}
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}

// This method is called after the window becomes active.
void WinApp::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			try
			{
				if (m_initComplete && m_windowVisible)
				{
					m_main->update();
					m_main->render();
				}
			}
			catch (std::exception& ex)
			{
				MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
				MigUtil::dumpLogToFile();
				m_windowClosed = true;
			}
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void WinApp::Uninitialize()
{
	MigUtil::suspendWatchdog();

	try
	{
		if (m_main != nullptr)
		{
			m_main->onDestroyGraphics();
			m_main->onDestroy();
		}
		m_main.reset();

		MigTech::MigGame::termRenderer();
		MigTech::MigGame::termGameEngine();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
	}

	m_initComplete = false;
}

// Application lifecycle event handlers.

void WinApp::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void WinApp::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.
		try
		{
			m_main->onSuspending();
		}
		catch (std::exception& ex)
		{
			MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
			MigUtil::dumpLogToFile();
			m_windowClosed = true;
		}

		deferral->Complete();
	});
}

void WinApp::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	try
	{
		m_main->onResuming();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}

	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// Insert your code here.
}

// Window event handlers.

void WinApp::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (!m_windowVisible)
		MigUtil::suspendWatchdog();

	try
	{
		m_main->onVisibilityChanged(m_windowVisible);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}

void WinApp::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;

	// cancel the watchdog
	if (m_timerWatchdog != nullptr)
		m_timerWatchdog->Cancel();
}

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
void WinApp::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	try
	{
		((MigTech::DxRender*) MigTech::MigUtil::theRend)->SetLogicalSize(MigTech::Size(sender->Bounds.Width, sender->Bounds.Height));
		m_main->onWindowSizeChanged();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}
#endif

// Mouse/touch/key event handlers

void WinApp::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	float x = args->CurrentPoint->Position.X / sender->Bounds.Width;
	float y = args->CurrentPoint->Position.Y / sender->Bounds.Height;
	try
	{
		m_main->onPointerPressed(x, y);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
	args->Handled = true;
}

void WinApp::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	float x = args->CurrentPoint->Position.X / sender->Bounds.Width;
	float y = args->CurrentPoint->Position.Y / sender->Bounds.Height;
	try
	{
		m_main->onPointerReleased(x, y);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
	args->Handled = true;
}

void WinApp::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	float x = args->CurrentPoint->Position.X / sender->Bounds.Width;
	float y = args->CurrentPoint->Position.Y / sender->Bounds.Height;
	try
	{
		m_main->onPointerMoved(x, y, args->CurrentPoint->IsInContact);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
	args->Handled = true;
}

void WinApp::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
	if (args->VirtualKey != m_lastKeyDown)
	{
		try
		{
			m_main->onKeyDown((VIRTUAL_KEY)args->VirtualKey);
		}
		catch (std::exception& ex)
		{
			MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
			MigUtil::dumpLogToFile();
			m_windowClosed = true;
		}
		m_lastKeyDown = args->VirtualKey;
	}
	args->Handled = true;
}

void WinApp::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
	try
	{
		m_main->onKeyUp((VIRTUAL_KEY)args->VirtualKey);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
	args->Handled = true;
}

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
void WinApp::OnBackKey(Platform::Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs^ args)
{
	try
	{
		args->Handled = m_main->onBackKey();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}
#endif

// DisplayInformation event handlers.

void WinApp::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	// NOT SURE WHAT THIS IS
	//m_deviceResources->ValidateDevice();
}

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
void WinApp::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	try
	{
		((MigTech::DxRender*) MigTech::MigUtil::theRend)->SetDpi(sender->LogicalDpi);
		m_main->onWindowSizeChanged();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}
#endif

void WinApp::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	try
	{
		((MigTech::DxRender*) MigTech::MigUtil::theRend)->SetCurrentOrientation(sender->CurrentOrientation);
		m_main->onWindowSizeChanged();
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		m_windowClosed = true;
	}
}
