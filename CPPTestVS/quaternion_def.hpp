#pragma once

#include "vector_def.hpp"

namespace Math
{
	struct Quaternion
	{
		union
		{
			struct { float x, y, z, w; };
			Vector<4, float> Points;
			struct { Vector<3, float> Vec; float Scal; };
		};

		inline Quaternion(float p_x, float p_y, float p_z, float p_w) : x{p_x}, y{p_y}, z{p_z}, w{p_w}
		{
			
		}

		inline Quaternion(const Vector<4, float>& p_points) : Points{p_points}
		{
		}

		inline Quaternion(const Vector<3, float>& p_vec, float p_scal) : Vec{p_vec}, Scal{p_scal}
		{
		}
	};
}