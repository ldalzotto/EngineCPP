#pragma once

#include "math.hpp"
#include "geometry_def.hpp"
#include "transform_def.hpp"

using namespace Math;

struct Geometry
{
	template<class TYPE>
	inline static bool overlap(const AABB<TYPE>& p_left, const AABB<TYPE>& p_right)
	{
		if (fabsf(p_left.center.x - p_right.center.x) > (p_left.radiuses.x + p_right.radiuses.x)) return false;
		if (fabsf(p_left.center.y - p_right.center.y) > (p_left.radiuses.y + p_right.radiuses.y)) return false;
		if (fabsf(p_left.center.z - p_right.center.z) > (p_left.radiuses.z + p_right.radiuses.z)) return false;

		return true;
	};

	template<class TYPE>
	inline static AABB<TYPE> project(const AABB<TYPE>& p_left, const Transform& p_transform)
	{
		return AABB<float>(
			Math::add(p_left.center, Math::add(p_transform.position, Math::mul(p_left.center, p_transform.scale))),
			Math::mul(p_left.radiuses, p_transform.scale)
		);
	};

	template<class TYPE>
	inline static AABB<TYPE> project(const AABB<TYPE>& p_left, const Matrix<4, TYPE>& p_trs)
	{
		AABB<TYPE> l_return;
		l_return.center = mul(p_trs, Vector<4, float>(p_left.center, 1.0f)).Vec3;
		l_return.radiuses = min(
			mul(p_trs, Vector<4, float>(p_left.radiuses, 1.0f)).Vec3,
			mul(p_trs, Vector<4, float>(VecConst<float>::ZERO, 1.0f)).Vec3
		);
		return l_return;
	};

	/*
	template<class TYPE>
	inline static bool overlap(const OBB<TYPE>& p_left, const OBB<TYPE>& p_right)
	{
		//TODO - Project right to left
	};
	*/
};
