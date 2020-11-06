#pragma once

#include "Common/Include/platform_include.hpp"

struct Clock
{
	size_t framecount = 0;
	float deltatime = 0;

	inline void newframe()
	{
		this->framecount += 1;
	}

	inline void newupdate(float p_delta)
	{
		this->deltatime = p_delta;
	}
};

typedef unsigned long long int TimeClockPrecision;

#ifdef _WIN32

#include <sysinfoapi.h>

inline TimeClockPrecision clock_currenttime_mics()
{
	FILETIME l_currentTime;
	GetSystemTimeAsFileTime(&l_currentTime);
	ULARGE_INTEGER ul;
	ul.LowPart = l_currentTime.dwLowDateTime;
	ul.HighPart = l_currentTime.dwHighDateTime;
	TimeClockPrecision l_time = ul.QuadPart;
	l_time /= 10;
	return l_time;
};

#endif // _WIN32


