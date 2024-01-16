#pragma once

// �ִ���Win32�ؼ�
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "resource.h"
#include "src/Snow.h"
#include "src/Board.h"
#include "src/Dialog.h"
#include "src/Thread_pool.h"
#include "src/GameData.h"
#include "include/httplib/httplib.h"
#include "include/nlohmann/json.hpp"
#include "include/GameEngine/GameEngine.h"
using json = nlohmann::json;

// ����Ϸ�İ汾�������⵽��̨�İ汾�ű����󣬾͸ø�����
#define VERSION 5

#ifndef _DEBUG
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )	// ���ؿ���̨
#endif // _DEBUG

#pragma comment(lib, "Imm32.lib")

// string -> wstring��ע�⣡�˺�����c++17�в����� ��Ҫ��� #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING �궨�壩
static std::wstring to_wide_string(const std::string& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(input);
}
// wstring -> string��ע�⣡�˺�����c++17�в����� ��Ҫ��� #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING �궨�壩
static std::string to_byte_string(const std::wstring& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(input);
}