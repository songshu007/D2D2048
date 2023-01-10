#pragma once
// 以免 Win32 的控件是 Win98 样式的
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

// 该游戏的版本，如果检测到后台的版本号比它大，就该更新了
#define VERSION 5

#ifndef _DEBUG
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )	// 隐藏控制台
#endif // _DEBUG

// string -> wstring（注意！此函数在c++17中不可用 需要添加 #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 宏定义）
static std::wstring to_wide_string(const std::string& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(input);
}
// wstring -> string（注意！此函数在c++17中不可用 需要添加 #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 宏定义）
static std::string to_byte_string(const std::wstring& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(input);
}