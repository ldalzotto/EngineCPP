#pragma once

#include <math.h>
#include "contants.hpp"

namespace Math
{
	template<class TYPE>
	inline bool Equals(const TYPE& p_left, const TYPE& p_right);

	template<>
	inline bool Equals<float>(const float& p_left, const float& p_right)
	{
		return fabsf(p_left - p_right) <= Tolerance<float>::tol;
	}
}
