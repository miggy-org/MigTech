///////////////////////////////////////////////////////////////////////////
// platform specific

#include "pch.h"
#include <windowsx.h>
#include "../core/MigInclude.h"
#include "../core/PersistBase.h"
#include "../windows/RegistryPersist.h"
#include "DesktopApp.h"
#include "DxRender.h"
//#include "XAudio.h"
#include "LegacySound.h"
#include "AppResource.h"

using namespace MigTech;
using namespace tinyxml2;

// defined in Platform.cpp
extern std::vector<std::string> dtContentFolders;
extern std::vector<std::string> dtShaderFolders;

// singleton
static DesktopApp theApp;

///////////////////////////////////////////////////////////////////////////
// app entry point

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(theApp.InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(theApp.InitDevice()))
	{
		theApp.CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (theApp.isVisible())
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				theApp.Run();
			}
		}
		else
		{
			if (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	theApp.CleanupDevice();

	return (int)msg.wParam;
}

///////////////////////////////////////////////////////////////////////////
// WinApp

DesktopApp::DesktopApp() :
	m_hInst(NULL),
	m_hWnd(NULL),
	m_initComplete(false),
	m_windowVisible(true),
	m_currSizeIndex(0)
{
}

#define ID_BASE_SIZES	1000

static HKEY OpenAppKey(bool read)
{
	HKEY hKey;
	LONG ret = ::RegCreateKeyEx(
		HKEY_CURRENT_USER,
		L"Software\\Jordan Enterprises\\MigTech",
		0, NULL,
		REG_OPTION_NON_VOLATILE,
		(read ? KEY_QUERY_VALUE : KEY_SET_VALUE),
		NULL, &hKey, NULL);
	return hKey;
}

// called every time the application receives a message
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		theApp.OnCommand(wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		theApp.OnLButtonDown(lParam);
		break;

	case WM_LBUTTONUP:
		theApp.OnLButtonUp(lParam);
		break;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON)
			theApp.OnMouseMove(lParam);
		break;

	case WM_KEYDOWN:
		theApp.OnKeyDown((VIRTUAL_KEY)wParam);
		break;

	case WM_KEYUP:
		theApp.OnKeyUp((VIRTUAL_KEY)wParam);
		break;

	case WM_SIZE:
		theApp.OnSize(wParam, lParam);
		break;

	case WM_WINDOWPOSCHANGED:
		theApp.OnPosChanged(wParam, lParam);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MUSICEVENT:
		MigTech::ProcessMusicEvent();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool DesktopApp::loadConfigXml()
{
	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument("desktop.xml");
	if (pdoc != nullptr)
	{
		XMLElement* desktop = pdoc->FirstChildElement("Desktop");
		if (desktop != nullptr)
		{
			XMLElement* elem = desktop->FirstChildElement("Content");
			if (elem != nullptr)
			{
				const char* p = elem->GetText();
				dtContentFolders.push_back(p);
			}

			elem = desktop->FirstChildElement("Shaders");
			while (elem != nullptr)
			{
				const char* p = elem->GetText();
				dtShaderFolders.push_back(p);

				elem = elem->NextSiblingElement("Shaders");
			}

			elem = desktop->FirstChildElement("Res");
			while (elem != nullptr)
			{
				char szTmp[80];
				const char* p = elem->GetText();
				if (p != nullptr)
				{
					strcpy_s(szTmp, 80, p);

					char* context;
					const char* pw = strtok_s(szTmp, " x", &context);
					const char* ph = strtok_s(nullptr, " x", &context);

					SIZE size;
					size.cx = atoi(pw);
					size.cy = atoi(ph);
					m_sizes.push_back(size);
				}

				elem = elem->NextSiblingElement("Res");
			}
		}

		delete pdoc;
	}
	else
	{
		LOGWARN("(DesktopApp::loadConfigXml) Unable to load config script");
		dtContentFolders.push_back("content\\");
		dtShaderFolders.push_back("shaders\\");
	}

	return true;
}

bool DesktopApp::loadRegistryValues(int& left, int& top)
{
	left = top = CW_USEDEFAULT;

	// retrieve saved size index
	HKEY hKey = OpenAppKey(true);
	if (hKey != NULL)
	{
		DWORD dwSize = sizeof(DWORD);
		::RegQueryValueEx(hKey, L"SizeIndex", 0, NULL, (LPBYTE)&m_currSizeIndex, &dwSize);
		::RegQueryValueEx(hKey, L"Left", 0, NULL, (LPBYTE)&left, &dwSize);
		::RegQueryValueEx(hKey, L"Top", 0, NULL, (LPBYTE)&top, &dwSize);
		::RegCloseKey(hKey);
	}
	return (hKey != NULL);
}

static const wchar_t* composeMenuSizeName(const SIZE& size)
{
	static wchar_t szTmp[80];
	swprintf_s(szTmp, 80, L"%d x %d", size.cx, size.cy);
	return szTmp;
}

HRESULT DesktopApp::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	if (!loadConfigXml())
		return E_FAIL;

	int left, top;
	if (!loadRegistryValues(left, top))
		return E_FAIL;

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_MIGTECH);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MIGTECH);
	wcex.lpszClassName = L"MigTechWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_MIGTECH);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	m_hInst = hInstance;
	RECT rc = { 0, 0, getCurrentSize().cx, getCurrentSize().cy };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
	m_hWnd = CreateWindow(L"MigTechWindowClass", L"MigTech",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		left, top, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!m_hWnd)
		return E_FAIL;

	// create the sizes menu
	HMENU hMenu = GetMenu(m_hWnd);
	if (hMenu != NULL)
	{
		HMENU hPopup = CreatePopupMenu();
		for (int i = 0; i < (int) m_sizes.size(); i++)
		{
			AppendMenu(hPopup, MF_STRING, ID_BASE_SIZES + i, composeMenuSizeName(m_sizes[i]));
		}
		AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR) hPopup, L"&Size");
	}

	ShowWindow(m_hWnd, nCmdShow);
	return S_OK;
}

HRESULT DesktopApp::InitDevice()
{
	HRESULT res = S_OK;

	try
	{
		// initialize MigTech
		//MigTech::AudioBase* pAudio = new MigTech::XAudioManager();
		MigTech::AudioBase* pAudio = new MigTech::LegacyAudioManager(m_hInst, m_hWnd);
		MigTech::PersistBase* pDataManager = new MigTech::RegistryPersist();
		MigTech::MigGame::initGameEngine(pAudio, pDataManager);
		MigTech::RenderBase* pRtObj = new MigTech::DxRender();
		MigTech::MigGame::initRenderer(pRtObj);

		// load the app name
		std::string appName = MigTech::MigUtil::getString("app", "MigTech");
		::SetWindowTextA(m_hWnd, appName.c_str());

		// pass the window to the renderer
		((MigTech::DxRender*) MigTech::MigUtil::theRend)->SetWindow(m_hWnd);

		// create the game class, now that MigTech is ready
		m_main = std::unique_ptr<MigTech::MigGame>(allocGame());
		if (m_main != nullptr)
		{
			m_main->onCreate();
			m_main->onCreateGraphics();
			m_main->onWindowSizeChanged();

			m_initComplete = true;
		}
		else
			res = E_FAIL;
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		PostQuitMessage(0);
		res = E_FAIL;
	}

	return res;
}

void DesktopApp::CleanupDevice()
{
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

void DesktopApp::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// these are the loaded window sizes
	if (LOWORD(wParam) >= ID_BASE_SIZES && LOWORD(wParam) < ID_BASE_SIZES + m_sizes.size())
	{
		int index = LOWORD(wParam) - ID_BASE_SIZES;
		const SIZE& size = m_sizes[index];
		LOGINFO("(DesktopApp::OnCommand) App size changing to %dx%d", size.cx, size.cy);

		// adjust to compensate for non-client area
		RECT rc = { 0, 0, size.cx, size.cy };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
		SetWindowPos(m_hWnd, NULL, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOMOVE | SWP_NOZORDER);
	}
	else if (LOWORD(wParam) == IDM_RESTART)
	{
		LOGINFO("(DesktopApp::OnCommand) App is restarting");
		try
		{
			if (m_main != nullptr)
			{
				m_main->onDestroyGraphics();
				m_main->onDestroy();
			}
			m_main.reset();
			m_initComplete = false;

			m_main = std::unique_ptr<MigTech::MigGame>(allocGame());
			if (m_main != nullptr)
			{
				m_main->onCreate();
				m_main->onCreateGraphics();
				m_main->onWindowSizeChanged();

				m_initComplete = true;
			}
		}
		catch (std::exception& ex)
		{
			MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
			MigUtil::dumpLogToFile();
			PostQuitMessage(0);
		}

	}
	else if (LOWORD(wParam) == IDM_EXIT)
	{
		LOGINFO("(DesktopApp::OnCommand) App is exiting");
		PostQuitMessage(0);
	}
}

void DesktopApp::processSizeChange(int newWidth, int newHeight)
{
	// find the menu item index the size matches
	HMENU hMenu = GetSubMenu(GetMenu(m_hWnd), 1);
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	for (int i = 0; i < (int)m_sizes.size(); i++)
	{
		if (newWidth == m_sizes[i].cx && newHeight == m_sizes[i].cy)
		{
			mii.fState = MFS_CHECKED;
			m_currSizeIndex = i;
		}
		else
			mii.fState = MFS_UNCHECKED;

		// update the menu check boxes
		SetMenuItemInfo(hMenu, i, TRUE, &mii);
	}

	if (isVisible() && MigUtil::theRend != nullptr)
	{
		// save size index (only if it changed, not before the renderer is initialized)
		HKEY hKey = OpenAppKey(false);
		if (hKey != NULL)
		{
			::RegSetValueEx(hKey, L"SizeIndex", 0, REG_DWORD, (LPBYTE)&m_currSizeIndex, sizeof(DWORD));
			::RegCloseKey(hKey);
		}

		MigTech::Size theSize = MigUtil::theRend->getOutputSize();
		if (newWidth != (int)theSize.width || newHeight != (int)theSize.height)
		{
			theSize = MigTech::Size((float)newWidth, (float)newHeight);

			try
			{
				((DxRender*)MigUtil::theRend)->SetLogicalSize(theSize);
				m_main->onWindowSizeChanged();
			}
			catch (std::exception& ex)
			{
				MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
				MigUtil::dumpLogToFile();
				PostQuitMessage(0);
			}
		}
	}
}

void DesktopApp::OnSize(WPARAM wParam, LPARAM lParam)
{
	//setVisible(wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE ? false : true);
	processSizeChange(LOWORD(lParam), HIWORD(lParam));
}

void DesktopApp::OnPosChanged(WPARAM wParam, LPARAM lParam)
{
	WINDOWPOS* wpos = (WINDOWPOS*)lParam;
	if (!(wpos->flags & (SWP_NOMOVE | SWP_HIDEWINDOW)) && wpos->x >= 0 && wpos->y >= 0)
	{
		// save the position
		HKEY hKey = OpenAppKey(false);
		if (hKey != NULL)
		{
			::RegSetValueEx(hKey, L"Left", 0, REG_DWORD, (LPBYTE)&wpos->x, sizeof(DWORD));
			::RegSetValueEx(hKey, L"Top", 0, REG_DWORD, (LPBYTE)&wpos->y, sizeof(DWORD));
			::RegCloseKey(hKey);
		}
	}
	else if (wpos->flags & SWP_SHOWWINDOW)
	{
		setVisible(true);
	}
	else if (wpos->flags & SWP_HIDEWINDOW)
	{
		setVisible(false);
	}
	else if (!(wpos->flags & SWP_NOSIZE))
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		int newWidth = rect.right - rect.left;
		int newHeight = rect.bottom - rect.top;
		if (newWidth > 0 && newHeight > 0)
			processSizeChange(newWidth, newHeight);
	}
}

void DesktopApp::OnLButtonDown(LPARAM lParam)
{
	float x = GET_X_LPARAM(lParam) / (float)getCurrentSize().cx;
	float y = GET_Y_LPARAM(lParam) / (float)getCurrentSize().cy;
	try
	{
		m_main->onPointerPressed(x, y);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		PostQuitMessage(0);
	}
}

void DesktopApp::OnLButtonUp(LPARAM lParam)
{
	float x = GET_X_LPARAM(lParam) / (float)getCurrentSize().cx;
	float y = GET_Y_LPARAM(lParam) / (float)getCurrentSize().cy;
	try
	{
		m_main->onPointerReleased(x, y);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		PostQuitMessage(0);
	}
}

void DesktopApp::OnMouseMove(LPARAM lParam)
{
	float x = GET_X_LPARAM(lParam) / (float)getCurrentSize().cx;
	float y = GET_Y_LPARAM(lParam) / (float)getCurrentSize().cy;
	try
	{
		m_main->onPointerMoved(x, y, true);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		PostQuitMessage(0);
	}
}

void DesktopApp::OnKeyDown(VIRTUAL_KEY key)
{
	if (key != m_lastKeyDown)
	{
		try
		{
			m_main->onKeyDown((VIRTUAL_KEY)key);
		}
		catch (std::exception& ex)
		{
			MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
			MigUtil::dumpLogToFile();
			PostQuitMessage(0);
		}
		m_lastKeyDown = key;
	}
}

void DesktopApp::OnKeyUp(VIRTUAL_KEY key)
{
	try
	{
		m_main->onKeyUp((VIRTUAL_KEY)key);
	}
	catch (std::exception& ex)
	{
		MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigUtil::dumpLogToFile();
		PostQuitMessage(0);
	}
}

void DesktopApp::Run()
{
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
		PostQuitMessage(0);
	}
}
