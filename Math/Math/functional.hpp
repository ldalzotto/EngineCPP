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

	template<class TYPE>
	inline short int sign(const TYPE& p_value)
	{
		if (p_value <= -Tolerance<TYPE>::tol)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}

	template<class TYPE>
	inline TYPE clamp(const TYPE& p_value, const TYPE& p_left, const TYPE& p_right)
	{
		if (p_value >= (p_right + Tolerance<TYPE>::tol))
		{
			return p_right;
		}
		else if (p_value <= (p_left + Tolerance<TYPE>::tol))
		{
			return p_left;
		}
		return p_value;
	}
}
