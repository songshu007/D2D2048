#pragma once
#include "Direct2dApp/Direct2dApp.h"
#include <iostream>
#include <shobjidl_core.h>
#include <string>
#include "Board.h"
#include "resource.h"
#pragma comment(lib, "Winmm.lib")

class AppWindow
{
public:
	AppWindow();
	~AppWindow();

	int Run(HINSTANCE hInstance);
	bool createWindow();
	void MessageLoop();
	void saveTo();
	float getFPS();
	void init(HWND hwnd);
	void Render();

	static DWORD WINAPI ThreadProc(
		LPVOID lpParameter
	);
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WindowDiglogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	bool Vertical_sync = true;
	bool Ai_Move = false;
	bool Animation_switch = true;
	static AppWindow* sInstance;
	app::Direct2dApp App;
	HINSTANCE hInstance;
	HWND m_hwnd;
	Board m_board;
	DWORD m_board_width;	// ∆Â≈Ã øÌ
	DWORD m_board_height;	// ∆Â≈Ã ∏ﬂ

	DIR m_Ai_Move = UP;
	Clock m_clock;
};
