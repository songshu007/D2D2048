#include "Clock.h"

Clock::Clock()
	:base_time(0)
{
	base_time = timeGetTime();
}

Clock::~Clock()
{

}

float Clock::getTime()
{
	return (timeGetTime() - base_time) / 1000.0f;
}

void Clock::Reset()
{
	base_time = timeGetTime();
}