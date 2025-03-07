// fontmaker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "fontmaker.h"

// Global Variables:
HINSTANCE hInst;						// current instance
TCHAR szTitle[MAX_PATH];				// The title bar text
TCHAR szWindowClass[MAX_PATH];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// defined in worker.cpp
extern bool ParseInputXML(const wchar_t* xmlFile);
extern bool CreateBitmapFromSelectedFont(LPLOGFONT lf, HWND hWnd);
extern void ResizeWindowToMatchBitmap(HWND hWnd);
extern void PaintBitmap(HDC hDC);
extern bool SaveFontImageAndConfiguration(const wchar_t* name);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_PATH);
	LoadString(hInstance, IDC_FONTMAKER, szWindowClass, MAX_PATH);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FONTMAKER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FONTMAKER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static void RestoreLastScript(HWND hWnd)
{
	// retrieve last used script
	HKEY hKey;
	LONG ret = ::RegCreateKeyEx(
		HKEY_CURRENT_USER,
		L"Software\\Jordan Enterprises\\MigTech\\FontMaker",
		0, NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_QUERY_VALUE,
		NULL, &hKey, NULL);
	if (ret == ERROR_SUCCESS)
	{
		wchar_t szLastScript[MAX_PATH] = { 0 };
		DWORD dwSize = sizeof(szLastScript);
		ret = ::RegQueryValueEx(hKey, L"LastScript", 0, NULL, (LPBYTE)szLastScript, &dwSize);
		::RegCloseKey(hKey);

		if (ret == ERROR_SUCCESS && szLastScript[0])
		{
			if (ParseInputXML(szLastScript))
			{
				ResizeWindowToMatchBitmap(hWnd);
				EnableMenuItem(GetMenu(hWnd), IDM_SELECT_FONT, MF_ENABLED);
			}
		}
	}
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   RestoreLastScript(hWnd);
   return TRUE;
}

static const wchar_t* PickFileDialog(HWND hWnd)
{
	static wchar_t szBuf[MAX_PATH] = { 0 };

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("XML Files (*.xml)\0*.xml\0\0");
	ofn.lpstrTitle = _T("Locate input script");
	ofn.lpstrFile = szBuf;
	ofn.nMaxFile = ARRAYSIZE(szBuf);
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER;

	BOOL bOK = GetOpenFileName(&ofn);
	if (bOK)
	{
		// save script
		HKEY hKey;
		LONG ret = ::RegCreateKeyEx(
			HKEY_CURRENT_USER,
			L"Software\\Jordan Enterprises\\MigTech\\FontMaker",
			0, NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE,
			NULL, &hKey, NULL);
		if (ret == ERROR_SUCCESS)
		{
			ret = ::RegSetValueEx(hKey, L"LastScript", 0, REG_SZ, (LPBYTE)szBuf, (wcslen(szBuf) + 1)*sizeof(wchar_t));
			::RegCloseKey(hKey);
		}
	}

	return (bOK ? szBuf : nullptr);
}

// note - currently only TT fonts, need to expand later
static LPLOGFONT PickFontDialog(HWND hWnd)
{
	static CHOOSEFONT cf = { 0 };
	static LOGFONT lf = { 0 };

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_FORCEFONTEXIST | CF_NOSIZESEL | CF_NOVERTFONTS | CF_SCALABLEONLY | CF_TTONLY | CF_INITTOLOGFONTSTRUCT;

	return (ChooseFont(&cf) ? &lf : nullptr);
}

static const wchar_t* PickSaveDialog(HWND hWnd)
{
	static wchar_t szBuf[MAX_PATH] = { 0 };

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = _T("PNG Files (*.png)\0*.png\0\0");
	ofn.lpstrTitle = _T("Choose font name");
	ofn.lpstrFile = szBuf;
	ofn.nMaxFile = ARRAYSIZE(szBuf);
	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	return (GetSaveFileName(&ofn) ? szBuf : nullptr);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		// Parse the menu selections
		switch (LOWORD(wParam))
		{
		case IDM_OPEN_SCRIPT:
			if (ParseInputXML(PickFileDialog(hWnd)))
			{
				ResizeWindowToMatchBitmap(hWnd);
				EnableMenuItem(GetMenu(hWnd), IDM_SELECT_FONT, MF_ENABLED);
			}
			else
				MessageBox(hWnd, L"Parsing error in script", L"OOPS", MB_OK | MB_ICONERROR);
			break;

		case IDM_SELECT_FONT:
			if (CreateBitmapFromSelectedFont(PickFontDialog(hWnd), hWnd))
			{
				ResizeWindowToMatchBitmap(hWnd);

				HMENU hMenu = GetMenu(hWnd);
				if (hMenu != NULL)
					EnableMenuItem(hMenu, IDM_SAVE_PACKAGE, MF_ENABLED);
			}
			//else
			//	MessageBox(hWnd, L"Error creating font bitmap", L"OOPS", MB_OK | MB_ICONERROR);
			break;

		case IDM_SAVE_PACKAGE:
			if (SaveFontImageAndConfiguration(PickSaveDialog(hWnd)))
				MessageBox(hWnd, L"Font package saved", L"Save Font Package", MB_OK | MB_ICONINFORMATION);
			else
				MessageBox(hWnd, L"Font package failed to save", L"Save Font Package", MB_OK | MB_ICONERROR);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		PaintBitmap(hdc);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
