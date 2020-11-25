#pragma once

#include "vector_def.hpp"

#include <string.h>

struct StringSlice
{
	const char* Memory;
	size_t Begin;
	size_t End;

	inline StringSlice()
	{
		this->Memory = nullptr;
		this->Begin = 0;
		this->End = 0;
	};

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

	bool equals(const StringSlice& p_other) const;

	inline size_t size() { return this->End - this->Begin; }
};

template<class Allocator = HeapAllocator>
struct String
{
	com::Vector<char, Allocator> Memory;

	void allocate(size_t p_initialSize, const Allocator& p_allocator = Allocator());
	void from_raw(char* p_str);
	void free();
	String<Allocator> clone() const;

	inline char* c_str() const { return this->Memory.Memory; }

	String<Allocator>& append(const char* p_str);
	template<class ParameterAllocator>
	String<Allocator>& append(const String<ParameterAllocator>& p_str);
	String<Allocator>& append(const StringSlice& p_slice);

	template<class ElementType>
	String<Allocator>& append(const ElementType& p_element);

	void clear();

	StringSlice toSlice() const;

	bool find(char* p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;
	template<class ParameterAllocator>
	bool find(const String<ParameterAllocator>& p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;
	bool find(const StringSlice& p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const;

	com::Vector<StringSlice> split(const char* p_char);

	bool equals(const char* p_str);
	bool equals(const StringSlice& p_str);

	void remove_chars(const char p_char);

	static char String_CompareRaw(char* p_left, char* p_right, size_t p_size);
};

