#pragma once

inline int8* heap_malloc(const uimax p_size)
{
	return (int8*)::malloc(p_size);
};


inline int8* heap_realloc(int8* p_memory, const uimax p_new_size)
{
	return (int8*)::realloc(p_memory, p_new_size);
};

inline void heap_free(int8* p_memory)
{
	::free(p_memory);
};


inline int8* memory_cpy(int8* p_dst, const int8* p_src, const uimax p_size)
{
	return (int8*)::memcpy(p_dst, p_src, p_size);
};

inline static int8* memory_cpy_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const  uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	if (p_size > p_dst_size)
	{
		abort();
	}
#endif

	return memory_cpy(p_dst, p_src, p_size);
};


inline int8* memory_move(int8* p_dst, const int8* p_src, const uimax p_size)
{
	return (int8*)::memmove(p_dst, p_src, p_size);
};


inline int8* memory_move_safe(int8* p_dst, const uimax p_dst_size, const int8* p_src, const uimax p_size)
{
#if STANDARD_ALLOCATION_BOUND_TEST
	if (p_size > p_dst_size)
	{
		abort();
	}
#endif

	return memory_move(p_dst, p_src, p_size);
};

inline int8 memory_compare(const int8* p_source, const int8* p_compared, const uimax p_size)
{
	return ::memcmp(p_source, p_compared, p_size) == 0;
};


template<class ElementType>
inline uimax memory_offset_bytes(const uimax p_size)
{
	return sizeof(ElementType) * p_size;
};

