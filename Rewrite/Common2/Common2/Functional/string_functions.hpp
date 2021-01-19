#pragma once

namespace v2
{
	struct FromString
	{
		inline static float32 afloat32(Slice<int8> p_string)
		{
			uimax l_dot_index;
			if (p_string.find(slice_int8_build_rawstr("."), &l_dot_index))
			{
				float32 l_return = 0.0f;
				for (loop(i, l_dot_index + 1, p_string.Size))
				{
					l_return += (float32)((p_string.get(i) - '0') * (1.0f / (pow(10, (double)i - l_dot_index))));
				}
				for (loop_reverse(i, l_dot_index - 1, -1))
				{
					l_return += (float32)((p_string.get(i) - '0') * (pow(10, l_dot_index - 1 - (double)i)));
				}
				return l_return;
			};

			abort();
		};
	};
}