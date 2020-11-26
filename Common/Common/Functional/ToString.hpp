#pragma once

#include <stdio.h>
#include "Common/Container/string.hpp"

template<class ElementType>
struct ToString
{
	inline static String<> to_str(const ElementType& p_element);
};

template<class ElementType>
struct FromString
{
	inline static ElementType from_str(const StringSlice& p_pstr);
};



template<>
struct ToString<size_t>
{
	inline static String<> to_str(const size_t& p_element)
	{
		String<> l_str; l_str.allocate(30);
		sprintf(l_str.Memory.Memory, "%lld", p_element);
		return l_str;
	};
};

template<>
struct FromString<size_t>
{
	inline static size_t from_str(const StringSlice& p_pstr)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_pstr);
		size_t l_return = (size_t)atoll(l_str.c_str());
		l_str.free();
		return l_return;
	};
};

template<>
struct FromString<float>
{
	inline static float from_str(const StringSlice& p_pstr)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_pstr);
		float l_return = (float)atof(l_str.c_str());
		l_str.free();
		return l_return;
	};
};
