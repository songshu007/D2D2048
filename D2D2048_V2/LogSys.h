#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <sstream>

class Loger
{
public:
	static Loger* sInstance;
	Loger() 
	{
		/*sInstance = this;
		outFile.open("LogFile.txt", std::ios_base::out | std::ios_base::app);
		outFile << "====================================================================\n";
		printfTime(); outFile << '\n';*/
	}
	~Loger() { outFile.close(); }
	void printfTime()
	{
		SYSTEMTIME t;
		GetLocalTime(&t);
		outFile << "TIME: " << t.wYear << '/' << t.wMonth << '/' << t.wDay << ' ' << t.wHour << ':' << t.wMinute << ':' << t.wSecond ;
	}
	static void Write(const char* text) { /*sInstance->outFile << text;*/ }
	static void Write(std::string text) { /*sInstance->outFile << text;*/ }
	static void WriteTime() { /*sInstance->printfTime();*/ }

private:
	std::ofstream outFile;
};

class Hresult
{
public:
	Hresult(const char* file, int line) : hr(S_OK), text(file), line(line) {}
	~Hresult() 
	{
	}

	HRESULT operator = (HRESULT vaule)
	{
		hr = vaule;
		std::stringstream ss;
		if (FAILED(hr))
		{
			SYSTEMTIME t;
			GetLocalTime(&t);
			ss << "FAILED: " << "  File: " << text << "  Line: " << line << "  Vaule: " << hr << "  [" << t.wHour << ':' << t.wMinute << ':' << t.wSecond << ']' << '\n';
			//Loger::Write(ss.str());
		}
		else
		{
			/*ss << "SUCCEEDED: " << "  File: " << text << "  Line: " << line << '\n';
			LOGGER.Write(ss.str());*/
		}
		return *this;
	}

	operator HRESULT()
	{
		return hr;
	}

private:
	HRESULT hr;
	int line;
	const char* text;
};

#define HR Hresult(__FILE__, __LINE__)

