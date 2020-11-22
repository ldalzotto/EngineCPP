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
	return FILETIME_to_mics(l_currentTime);
};

#endif // _WIN32


