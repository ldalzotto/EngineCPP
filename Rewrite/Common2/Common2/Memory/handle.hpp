#pragma once

struct Handle
{
	char* handle;
};

inline Handle handle_build_default()
{
	return Handle{ NULL };
};

inline void handle_reset(Handle* p_handle)
{
	*p_handle = handle_build_default();
};