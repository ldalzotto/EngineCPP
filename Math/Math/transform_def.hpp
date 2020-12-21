#pragma once

#include "vector_def.hpp"
#include "quaternion_def.hpp"

namespace Math
{
	struct Transform
	{
		vec3f position = VecConst<float>::ZERO;
		quat rotation = QuatConst::IDENTITY;
		vec3f scale = VecConst<float>::ONE;

		Transform() {};
		Transform(const vec3f& p_localposition, const quat& p_localrotation, const vec3f& p_localscale) : position{p_localposition},
			rotation{ p_localrotation }, scale{p_localscale} {};
	};
}