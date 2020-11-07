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
	};
}