#pragma once
#include <stdlib.h>
#include <cstring>

struct Mem
{
	inline static void* memcpy(void* p_dst, void const* p_src, size_t p_size)
	{
		return ::memcpy(p_dst, p_src, p_size);
	};

	inline static void* memcpy_safe(void* p_dst, size_t p_dst_size, void const* p_src, size_t p_size)
	{
#if STANDARD_ALLOCATION_BOUND_TEST
		if (p_size > p_dst_size)
		{
			abort();
		}
#endif

		return Mem::memcpy(p_dst, p_src, p_size);
	};

	inline static void* memmove(void* p_dst, void const* p_src, size_t p_size)
	{
		return ::memmove(p_dst, p_src, p_size);
	};

	inline static void* memmove_safe(void* p_dst, size_t p_dst_size, void const* p_src, size_t p_size)
	{
#if STANDARD_ALLOCATION_BOUND_TEST
		if (p_size > p_dst_size)
		{
			abort();
		}
#endif

		return Mem::memmove(p_dst, p_src, p_size);
	};

};