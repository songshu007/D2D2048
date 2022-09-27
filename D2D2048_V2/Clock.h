#include <iostream>
#include <Windows.h>

class Clock
{
public:
	Clock();
	~Clock();
	float getTime();
	void Reset();

private:
	DWORD base_time;
};