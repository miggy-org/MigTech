#pragma once

#include "pch.h"
#include "../core/MigGame.h"

///////////////////////////////////////////////////////////////////////////
// application specific

extern MigTech::MigGame* allocGame();

///////////////////////////////////////////////////////////////////////////
// platform specific

namespace MigTech
{
	class DesktopApp sealed
	{
	public:
		DesktopApp();

		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
		HRESULT InitDevice();
		void CleanupDevice();

		void OnCommand(WPARAM wParam, LPARAM lParam);
		void OnSize(WPARAM wParam, LPARAM lParam);
		void OnPosChanged(WPARAM wParam, LPARAM lParam);
		void OnLButtonDown(LPARAM lParam);
		void OnLButtonUp(LPARAM lParam);
		void OnMouseMove(LPARAM lParam);

		void OnKeyDown(VIRTUAL_KEY key);
		void OnKeyUp(VIRTUAL_KEY key);

		void Run();

		bool isVisible() const { return m_windowVisible; }
		void setVisible(bool vis) { m_windowVisible = vis; }

	protected:
		bool loadConfigXml();
		bool loadRegistryValues(int& left, int& top);
		void processSizeChange(int newWidth, int newHeight);
		const SIZE& getCurrentSize() const { return m_sizes[m_currSizeIndex]; }

	private:
		std::unique_ptr<MigTech::MigGame> m_main;
		std::vector<SIZE> m_sizes;
		int m_currSizeIndex;

		HINSTANCE m_hInst;
		HWND m_hWnd;

		bool m_initComplete;
		bool m_windowVisible;
		VIRTUAL_KEY m_lastKeyDown;
	};
}
