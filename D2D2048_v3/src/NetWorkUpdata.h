#pragma once
#include <vector>
#include <map>
#include <string>
#include <locale>
#include <codecvt>
#include "../include/httplib/httplib.h"
#include "../include/nlohmann/json.hpp"
using json = nlohmann::json;

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

class NetWorkUpdata
{
public:
	NetWorkUpdata(std::atomic<bool>& is_ok, json& json);
	~NetWorkUpdata();

	void Run();

private:
	std::atomic<bool>& is_ok;
	json& m_json;

};

