#pragma once
#include <Windows.h>
#include <iostream>
#include <CommCtrl.h>
#include <shellapi.h>
#include "GameData.h"
#include "../resource.h"

static INT_PTR Dlg_About(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	// 当点击超链接时的消息
	case WM_NOTIFY: 
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		if (pnmhdr == nullptr) break;
		switch (pnmhdr->code)
		{
		case NM_CLICK:
		case NM_RETURN:
		{
			// 获取点击控件的ID
			auto id = pnmhdr->idFrom;
			if (id == IDC_WORD_LINK)
				ShellExecuteW(NULL, L"open", L"http://117.50.181.42:4000/backstage/2048", NULL, NULL, SW_SHOWNORMAL);
			if (id == IDC_GITHUB_LINK)
				ShellExecuteW(NULL, L"open", L"https://github.com/songshu007/D2D2048", NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		}
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, wParam);
			break;
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			break;
		default: break;
		}
		break;
	}
	}

	return FALSE;
}

static INT_PTR Dlg_Size(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			int w, h;
			wchar_t buf[32];
			memset(buf, 0, 32);
			int WidthLen = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDIT_WIDTH));
			GetDlgItemTextW(hwnd, IDC_EDIT_WIDTH, buf, WidthLen + 1);
			w = _wtoi(buf);
			memset(buf, 0, 32);
			int HeightLen = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_EDIT_HEIGHT));
			GetDlgItemTextW(hwnd, IDC_EDIT_HEIGHT, buf, HeightLen + 1);
			h = _wtoi(buf);
			if (w <= 0 || h <= 0)
			{
				::MessageBoxW(hwnd, L"你这不是为难我吗？", L"别闹了", MB_OK);
				return TRUE;
			}
			else
			{
				GameData::Get().board->InitBoard(w, h);
			}
			EndDialog(hwnd, wParam);
			return TRUE;
		}
		case IDCANCEL:
			EndDialog(hwnd, wParam);
			break;
		default: break;
		}
		break;
	}
	}

	return FALSE;


}