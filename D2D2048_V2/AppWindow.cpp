#include "AppWindow.h"

AppWindow* AppWindow::sInstance = nullptr;

#define RENDER_EVENT (WM_USER + 1001)

DWORD WINAPI AppWindow::ThreadProc(
	LPVOID lpParameter
)
{
	HANDLE hEvent = (HANDLE)lpParameter;
	while (1)
	{
		if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}
		sInstance->Render();
	}
	return 0;
}

AppWindow::AppWindow()
	: m_board_width(4), m_board_height(4)
{
	sInstance = this;
}
AppWindow::~AppWindow()
{

}
int AppWindow::Run(HINSTANCE hInstance)
{
	this->hInstance = hInstance;
	if (!this->createWindow()) return 0;
	this->MessageLoop();
	return 0;
}
bool AppWindow::createWindow()
{
#ifdef _DEBUG
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CON", "r", stdin);//重定向输入流
	freopen_s(&stream, "CON", "w", stdout);//重定向输入流
#endif

	//HINSTANCE hInstance = GetModuleHandleW(NULL);
	WNDCLASSEXW wic;
	memset(&wic, 0, sizeof(wic));
	wic.cbSize = sizeof(WNDCLASSEX);
	wic.hCursor = LoadCursor(0, IDC_ARROW);	// 光标
	wic.hInstance = this->hInstance;
	wic.lpszClassName = __TEXT("Hello");//窗口类名
	wic.lpfnWndProc = WindowProc;//指明消息处理函数
	wic.lpszMenuName = (LPCWSTR)IDR_MENU1;
	wic.style = CS_HREDRAW | CS_VREDRAW;
	//wic.hIcon = LoadIcon(this->hInstance, (LPCWSTR)IDB_BITMAP1);

	if (!RegisterClassEx(&wic))
		return false;

	RECT rect;
	rect.right = 512;	// 客户区大小
	rect.bottom = 512;
	rect.left = 0;
	rect.top = 0;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);	// rect.right 和 rect.bottom 是客户区的大小，该函数根据客户区的大小帮助我们计算整个窗口的大小

	// WS_EX_TOPMOST 可将窗口置于其它窗口顶部
	this->m_hwnd = CreateWindowEx(0, __TEXT("Hello"), __TEXT("2048"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL/*附加消息*/);
	this->App.initD2d(m_hwnd);
	this->init(this->m_hwnd);
	//SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);
	return true;
}
void AppWindow::MessageLoop()
{
	MSG msg = { 0 };

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}
void AppWindow::saveTo()
{
	wchar_t timebuf[256] = { 0 };
	SYSTEMTIME time;
	GetLocalTime(&time);
	swprintf_s(timebuf, L"%d-%d-%d-%d-%d-%d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wMilliseconds);

	ComPtr<IFileSaveDialog> pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		pfd->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

		const COMDLG_FILTERSPEC rgSaveTypes[] = { {L"(*.png)", L"*.png"} };
		pfd->SetFileName(timebuf);
		pfd->SetDefaultExtension(L"png");
		pfd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);

		if (pfd->Show(NULL) == S_OK)
		{
			ComPtr<IShellItem> pitem = NULL;
			if (pfd->GetResult(&pitem) == S_OK && pitem)
			{
				LPWSTR resultptr = NULL;
				if (pitem->GetDisplayName(SIGDN_FILESYSPATH, &resultptr) == S_OK && resultptr)
				{
					ID2D1Bitmap1* buf;
					this->App.getTarget(&buf);
					this->App.saveBitmapToFile(buf, resultptr);
					buf->Release();
					std::wcout.imbue(std::locale("chs"));
					std::wcout << resultptr << std::endl;
				}
				pitem->Release();
			}
		}
		pfd->Release();
	}

}
float AppWindow::getFPS()
{
	static DWORD LastTime = 0;
	static DWORD NowTime = 0;

	static DWORD LastFPS = 0;
	static float FPS = 0;

	NowTime = timeGetTime();
	LastFPS++;
	if ((NowTime - LastTime) >= 1000)
	{
		FPS = (float)LastFPS / ((NowTime - LastTime) * 0.001f);
		LastTime = NowTime;
		LastFPS = 0;
	}
	return FPS;
}
LRESULT AppWindow::WindowDiglogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			int w, h;
			wchar_t buf[32];
			memset(buf, 0, 32);
			int WidthLen = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_WIDTH));
			GetDlgItemTextW(hwnd, IDC_WIDTH, buf, WidthLen + 1);
			w = _wtoi(buf);
			memset(buf, 0, 32);
			int HeightLen = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_HEIGHT));
			GetDlgItemTextW(hwnd, IDC_HEIGHT, buf, HeightLen + 1);
			h = _wtoi(buf);
			if (w <= 0 || h <= 0)
			{
				MessageBoxW(hwnd, L"你这不是为难我吗？", L"别闹了", MB_OK);
				return TRUE;
			}
			else
			{
				sInstance->m_board_width = w;
				sInstance->m_board_height = h;
			}
			EndDialog(hwnd, wParam);
			return TRUE;
		}
		break;
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			return TRUE;
		}
	}
	break;
	default: break;
	}
	return FALSE;
}
LRESULT AppWindow::WindowDiglogAboutProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_NETWORK:
		{
			system("start https://github.com/songshu007/D2D2048");
		}
		break;
		case IDOK:
			EndDialog(hwnd, wParam);
			break;
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			break;
		default: break;
		}
	}
	break;
	default: break;
	}
	return FALSE;
}
LRESULT AppWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_AGAIN:
			sInstance->m_board.Reset();
			break;
		case ID_TEST:
			sInstance->m_board.Test();
			break;
		case ID_BOARD_SIZE:
		{
			DialogBoxW(NULL, (LPCWSTR)IDD_DIALOG1, hwnd, (DLGPROC)WindowDiglogProc);
			sInstance->m_board.ReSize(sInstance->m_board_width, sInstance->m_board_height);
		}
		break;
		case ID_ABOUT:
		{
			DialogBoxW(NULL, (LPCWSTR)IDD_DIALOG2, hwnd, (DLGPROC)WindowDiglogAboutProc);
		}
		break;
		case ID_Vertical:
		{
			sInstance->Vertical_sync = !sInstance->Vertical_sync;
			sInstance->App.setVerticalSync(sInstance->Vertical_sync);
			HMENU h = GetMenu(hwnd);
			CheckMenuItem(h, ID_Vertical, sInstance->Vertical_sync ? MF_CHECKED : MF_UNCHECKED);
		}
			break;
		case ID_AI_MTKL:
		{
			sInstance->Ai_Move = !sInstance->Ai_Move;
			HMENU h = GetMenu(hwnd);
			CheckMenuItem(h, ID_AI_MTKL, sInstance->Ai_Move ? MF_CHECKED : MF_UNCHECKED);
		}
		break;
		case ID_ANIMATION:
		{
			sInstance->Animation_switch = !sInstance->Animation_switch;
			sInstance->m_board.setAnimation(sInstance->Animation_switch);
			HMENU h = GetMenu(hwnd);
			CheckMenuItem(h, ID_ANIMATION, sInstance->Animation_switch ? MF_CHECKED : MF_UNCHECKED);
		}
		break;
		default:
			break;
		}
	}
	break;
	case WM_CLOSE:
	{
		SetEvent(sInstance->hEvent);
		Sleep(50);
		PostQuitMessage(0);
	}
		return 0;
	case WM_SIZE:
	{
		sInstance->App.ChangeSize(LOWORD(lParam), HIWORD(lParam));
	}
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'V':
			sInstance->saveTo();
			break;
		case 'W':
			sInstance->m_board.Move(UP);
			break;
		case 'S':
			sInstance->m_board.Move(DOWN);
			break;
		case 'A':
			sInstance->m_board.Move(LEFT);
			break;
		case 'D':
			sInstance->m_board.Move(RIGHT);
			break;
		case VK_UP:
			sInstance->m_board.Move(UP);
			break;
		case VK_DOWN:
			sInstance->m_board.Move(DOWN);
			break;
		case VK_LEFT:
			sInstance->m_board.Move(LEFT);
			break;
		case VK_RIGHT:
			sInstance->m_board.Move(RIGHT);
			break;
		default: break;
		}
		break;
	case WM_TIMER:
	{
		if (wParam == RENDER_EVENT) sInstance->Render();
	}
	break;
	default:
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
void AppWindow::init(HWND hwnd)
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	srand(time(0));

	hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

	CreateThread(0, 0, ThreadProc, hEvent, 0, 0);

	//SetTimer(hwnd, RENDER_EVENT, 0, 0);

	this->App.setVerticalSync(Vertical_sync);
	
	this->m_board.initBoard(m_board_width, m_board_height);
}
void AppWindow::Render()
{
	if (Ai_Move)
	{
		if (m_clock.getTime() >= 0.5f)
		{
			m_Ai_Move = this->m_board.GetAiMove();
			this->m_board.Move(m_Ai_Move);
			m_clock.Reset();
		}
	}

	RECT rc = this->App.getSize();
	static wchar_t buf[256] = { 0 };
	swprintf_s(buf, L"2048 FPS: %.1f 分数: %d", this->getFPS(), this->m_board.getSocre());
	SetWindowTextW(this->m_hwnd, buf);

	this->App.BeginDraw();
	this->App.Clear(D2D1::ColorF(187 / 255., 173 / 255., 160 / 255.));
	
	this->m_board.Render(this->App, rc);

	this->App.EndDraw();
}