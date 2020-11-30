#pragma once

#include <stdio.h>
#include "Common/Container/string.hpp"

template<class ElementType>
struct ToString
{
	inline static String<> to_str(const ElementType& p_element)
	{
		String<> l_str; l_str.allocate(30);
		int l_char_nb = sprintf_s(l_str.Memory.Memory, l_str.Memory.capacity_in_bytes(), ToStringFormat<ElementType>::format, p_element);
		l_str.Memory.Size = l_char_nb + 1;
		return l_str;
	};
};

template<class ElementType>
struct FromString
{
	inline static ElementType from_str(const StringSlice& p_pstr)
	{
		String<> l_str; l_str.allocate(30);
		l_str.append(p_pstr);
		ElementType l_return = FromStringParser<ElementType>::parse(l_str.c_str());
		l_str.free();
		return l_return;
	};

};

template<class ElementType>
struct ToStringFormat {};

template<>
struct ToStringFormat<size_t>
{
	inline static const char* format = "%lld";
};
template<>
struct ToStringFormat<short>
{
	inline static const char* format = "%hd";
};
template<>
struct ToStringFormat<float>
{
	inline static const char* format = "%f";
};
template<>
struct ToStringFormat<int>
{
	inline static const char* format = "%i";
};

template<class ElementType>
struct FromStringParser {};

template<>
struct FromStringParser<size_t>
{
	inline static size_t parse(const char* p_str)
	{
		return (size_t)atoll(p_str);
	};
};

template<>
struct FromStringParser<float>
{
	inline static float parse(const char* p_str)
	{
		return (float)atof(p_str);
	};
};

template<>
struct FromStringParser<int>
{
	inline static int parse(const char* p_str)
	{
		return (int)atoi(p_str);
	};
};

template<>
struct FromStringParser<short>
{
	inline static short parse(const char* p_str)
	{
		return (short)atoi(p_str);
	};
};

template<>
struct FromStringParser<bool>
{
	inline static bool parse(const char* p_str)
	{
		bool l_return = false;
		StringSlice p_str_slice = StringSlice(p_str);
		if (p_str_slice.equals(StringSlice("true")))
		{
			l_return = true;
		}
		else if (p_str_slice.equals(StringSlice("false")))
		{
			l_return = false;
		}
		return l_return;
	};
};


template<>
struct FromStringParser<String<>>
{
	inline static String<> parse(const char* p_str)
	{
		String<> l_str; l_str.allocate(strlen(p_str));
		l_str.append(p_str);
		return l_str;
	};
};
