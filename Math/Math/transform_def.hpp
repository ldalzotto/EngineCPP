#pragma once

#include "vector_def.hpp"
#include "quaternion_def.hpp"

namespace Math
{
	struct Transform
	{
		vec3f local_position = VecConst<float>::ZERO;
		quat local_rotation = QuatConst::IDENTITY;
		vec3f local_scale = VecConst<float>::ONE;

		Transform() {};
		Transform(const vec3f& p_localposition, const quat& p_localrotation, const vec3f& p_localscale) : local_position{p_localposition},
			local_rotation{ p_localrotation }, local_scale{p_localscale} {};
	};
}