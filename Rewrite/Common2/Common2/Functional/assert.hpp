#pragma once

inline void assert_true(bool p_condition)
{
	if (!p_condition) { abort(); }
};

