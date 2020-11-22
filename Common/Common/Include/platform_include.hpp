#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

inline size_t FILETIME_to_mics(FILETIME& p_filetime)
{
	ULARGE_INTEGER ul;
	ul.LowPart = p_filetime.dwLowDateTime;
	ul.HighPart = p_filetime.dwHighDateTime;
	return ul.QuadPart / 10;
};

#endif

