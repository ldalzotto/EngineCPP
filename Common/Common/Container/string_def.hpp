#pragma once

#include "vector_def.hpp"


struct StringSlice
{
	const char* Memory;
	size_t Begin;
	size_t End;

	inline StringSlice(const char* p_memory, size_t p_begin, size_t p_end) {
		this->Memory = p_memory;
		this->Begin = p_begin;
		this->End = p_end;
	};

	inline StringSlice(const char* p_memory)
	{
		this->Memory = p_memory;
		this->Begin = 0;
		this->End = strlen(p_memory);
	};

	bool find(const StringSlice& p_other, size_t* p_outfoundIndex);
	bool equals(const StringSlice& p_other);
};

template<class Allocator = HeapAllocator>
struct String
{
	com::Vector<char, Allocator> Memory;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	void from_raw(char* p_str);
	void free();

	String<Allocator>& append(const char* p_str);
	template<class ParameterAllocator>
	String<Allocator>& append(const String<ParameterAllocator>& p_str);
	String<Allocator>& append(const StringSlice& p_slice);

	void clear();

	StringSlice toSlice() const;

	bool find(char* p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;
	template<class ParameterAllocator>
	bool find(const String<ParameterAllocator>& p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;
	bool find(const StringSlice& p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;

	bool equals(const char* p_str);
	bool equals(const StringSlice& p_str);
};

