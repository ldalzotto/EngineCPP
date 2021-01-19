#pragma once

inline void assert_true(int8 p_condition)
{
	if (!p_condition) { abort(); }
};

