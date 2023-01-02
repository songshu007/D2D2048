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
#ifdef _DEBUG
		sInstance = this;
		outFile.open("LogFile.txt", std::ios_base::out);
		outFile << "====================================================================\n";
		printfTime(); outFile << '\n';
#endif // DEBUG
	}
	~Loger() 
	{
#ifdef _DEBUG
		outFile.close(); 
#endif
	}
	void printfTime()
	{
#ifdef _DEBUG
		SYSTEMTIME t;
		GetLocalTime(&t);
		outFile << "TIME: " << t.wYear << '/' << t.wMonth << '/' << t.wDay << ' ' << t.wHour << ':' << t.wMinute << ':' << t.wSecond ;
#endif
	}
	static void Write(const char* text)
	{ 
#ifdef _DEBUG
		sInstance->outFile << text;
#endif
	}
	static void Write(std::string text)
	{ 
#ifdef _DEBUG
		sInstance->outFile << text;
#endif
	}
	static void WriteTime()
	{ 
#ifdef _DEBUG
		sInstance->printfTime();
#endif
	}

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
#ifdef _DEBUG
			SYSTEMTIME t;
			GetLocalTime(&t);
			ss << "FAILED: " << "  File: " << text << "  Line: " << line << "  Vaule: " << hr << "  [" << t.wHour << ':' << t.wMinute << ':' << t.wSecond << ']' << '\n';
			Loger::Write(ss.str());
#endif
		}
		else
		{
#ifdef _DEBUG
			//ss << "SUCCEEDED: " << "  File: " << text << "  Line: " << line << '\n';
			//Loger::Write(ss.str());
#endif
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

