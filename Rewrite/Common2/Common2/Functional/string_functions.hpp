#pragma once

/*
String<> l_str; l_str.allocate(30);
		l_str.append(p_pstr);
		ElementType l_return = FromStringParser<ElementType>::parse(l_str.c_str());
		l_str.free();
		return l_return;
*/

struct FromString
{
	inline static float afloat(Slice<char> p_string)
	{
		size_t l_dot_index;
		if(p_string.find(slice_char_build_rawstr("."), &l_dot_index))
		{
			float l_return = 0.0f;
			for (size_t i = l_dot_index + 1; i < p_string.Size; i++)
			{ 
				l_return += (float)((p_string.get(i) - '0') * (1.0f / (pow(10, (double)i - l_dot_index))));
			}
			for (size_t i = l_dot_index - 1; i != -1; --i)
			{
				l_return += (float)((p_string.get(i) - '0') * (pow(10, l_dot_index - 1 - (double)i)));
			}

			return l_return;
		};

		abort();
	};
};