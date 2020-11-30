#pragma once

#include "string_def.hpp"
#include "Common/Functional/ToString.hpp"

template<class Allocator>
inline void String<Allocator>::allocate(size_t p_initialSize, const Allocator& p_allocator)
{
	this->Memory.allocate(p_initialSize, p_allocator);
	this->Memory.push_back((char)NULL);
}
template<class Allocator>
inline void String<Allocator>::from_raw(char* p_str)
{
	this->Memory.Memory = p_str;
	this->Memory.Size = strlen(p_str) + 1;
};

template<class Allocator>
inline void String<Allocator>::free()
{
	this->Memory.free();
};

template<class Allocator>
inline String<Allocator> String<Allocator>::clone() const
{
	String<Allocator> l_clone;
	l_clone.Memory = this->Memory.clone();
	return l_clone;
};

template<class Allocator>
inline String<Allocator>& String<Allocator>::append(const char* p_str)
{
	com::MemorySlice<char> l_str = com::MemorySlice<char>(*p_str, strlen(p_str));
	if (this->Memory.Size >= 2)
	{
		this->Memory.insert_at(l_str, this->Memory.Size - 1);
	}
	else
	{
		this->Memory.insert_at(l_str, 0);
	}
	return *this;
}

template<class Allocator>
template<class ParameterAllocator>
inline String<Allocator>& String<Allocator>::append(const String<ParameterAllocator>& p_str)
{
	com::MemorySlice<char> l_str = com::MemorySlice<char>(*p_str.c_str(), p_str.Memory.Size - 1);
	if (this->Memory.Size >= 2)
	{
		this->Memory.insert_at(l_str, this->Memory.Size - 1);
	}
	else
	{
		this->Memory.insert_at(l_str, 0);
	}
	return *this;
}



template<class Allocator>
inline String<Allocator>& String<Allocator>::append(const StringSlice& p_slice)
{
	com::MemorySlice<char> l_str;
	l_str.Memory = p_slice.Memory;
	l_str.Begin = p_slice.Begin;
	l_str.End = p_slice.End;

	if (this->Memory.Size >= 2)
	{
		this->Memory.insert_at(l_str, this->Memory.Size - 1);
	}
	else
	{
		this->Memory.insert_at(l_str, 0);
	}
	return *this;
}

template<class Allocator>
template<class ElementType>
inline String<Allocator>& String<Allocator>::append(const ElementType& p_element)
{
	String<> l_tmp_str = ToString<ElementType>::to_str(p_element);
	String<Allocator>& l_return = this->append(StringSlice(l_tmp_str.c_str()));
	l_tmp_str.free();
	return l_return;
};

template<class Allocator>
String<Allocator>& String<Allocator>::remove(const size_t p_begin, const size_t p_end)
{
	size_t l_end = p_end;
	if (l_end >= this->Memory.Size - 1)
	{
		l_end = this->Memory.Size - 1;
	}

	this->Memory.erase_at(p_begin, l_end - p_begin);
	return *this;
};

template<class Allocator>
inline void String<Allocator>::clear()
{
	this->Memory.Size = 0;
	this->Memory.push_back((char)NULL);
}


template<class Allocator>
inline static char String<Allocator>::String_CompareRaw(char* p_left, char* p_right, size_t p_size)
{
	for (size_t i = 0; i < p_size; i++)
	{
		if (p_left[i] != p_right[i])
		{
			return 0;
		}
	}
	return 1;
};

template<class Allocator>
inline StringSlice String<Allocator>::toSlice() const
{
	return StringSlice(this->Memory.Memory, 0, this->Memory.Size - 1);
};

template<class Allocator>
template<class ParameterAllocator>
inline bool String<Allocator>::find(const String<ParameterAllocator>& p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const
{
	StringSlice l_slice;
	l_slice.Memory = p_str.Memory.Memory;
	l_slice.Begin = p_search_begin;
	l_slice.End = p_str.Memory.Size - 1;
	return this->toSlice().find(l_slice, p_outfoundIndex);
};

template<class Allocator>
inline bool String<Allocator>::find(char* p_str, const size_t p_search_begin, size_t* p_outfoundIndex) const
{
	StringSlice l_slice = this->toSlice();
	l_slice.Begin = p_search_begin;
	return l_slice.find(StringSlice(p_str), p_outfoundIndex);
}

template<class Allocator>
inline bool String<Allocator>::find(const StringSlice& p_compared_str, const size_t p_search_begin, size_t* p_outfoundIndex) const
{
	StringSlice l_slice = this->toSlice();
	l_slice.Begin = p_search_begin;
	return l_slice.find(p_compared_str, p_outfoundIndex);
}

template<class Allocator>
inline com::Vector<StringSlice> String<Allocator>::split(const char* p_char)
{
	com::Vector<StringSlice> l_match;

	size_t l_last_index = 0;
	size_t l_index = 0;
	while (this->find(p_char, l_index, &l_index))
	{
		l_match.push_back(StringSlice(this->Memory.Memory, l_last_index, l_index));
		l_index += 1;
		l_last_index = l_index;
	}
	l_match.push_back(StringSlice(this->Memory.Memory, l_last_index, this->Memory.Size - 1));

	return l_match;
};

template<class Allocator>
inline bool String<Allocator>::equals(const char* p_str)
{
	return StringSlice(p_str).equals(this->toSlice());
};

template<class Allocator>
inline bool String<Allocator>::equals(const StringSlice& p_str)
{
	return p_str.equals(this->toSlice());
};

template<class Allocator>
void String<Allocator>::remove_chars(const char p_char)
{
	for (size_t i = this->Memory.Size; i <= this->Memory.Size; i--)
	{
		if (this->Memory[i] == p_char)
		{
			this->Memory.erase_at(i, 1);
		}
	}
};

inline bool StringSlice::find(const StringSlice& p_other, size_t* p_outfoundIndex)
{
	size_t l_stringSlice_size = this->End - this->Begin;
	if (l_stringSlice_size > 0)
	{
		for (*p_outfoundIndex = this->Begin; *p_outfoundIndex < this->End; (*p_outfoundIndex)++)
		{
			size_t l_endIndex = (*p_outfoundIndex) + (p_other.End - p_other.Begin);
			if (l_endIndex > this->End)
			{
				break;
			}

			const char* l_pstringCompareBegin = this->Memory + *p_outfoundIndex;
			const char* l_compareStringBegin = p_other.Memory + p_other.Begin;
			if (*l_pstringCompareBegin == *l_compareStringBegin)
			{
				if (memcmp(l_pstringCompareBegin, l_compareStringBegin, sizeof(char) * (p_other.End - p_other.Begin)) == 0)
				{
					return true;
				}
			}
		}
	}

	return false;
};

inline bool StringSlice::equals(const StringSlice& p_other) const
{

	size_t l_sourceSlice_size = (this->End - this->Begin);
	size_t l_comparedSlice_size = (p_other.End - p_other.Begin);

	if (l_sourceSlice_size > 0 && l_comparedSlice_size > 0 && l_sourceSlice_size == l_comparedSlice_size)
	{
		return memcmp(this->Memory + this->Begin, p_other.Memory + p_other.Begin, l_comparedSlice_size * sizeof(char)) == 0;
	}

	return false;
};