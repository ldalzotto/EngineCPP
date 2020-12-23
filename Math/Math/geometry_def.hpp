#pragma once

#include "vector_def.hpp"
#include "matrix_def.hpp"

namespace Math
{

	template<unsigned N, class TYPE>
	struct Segment
	{
		Vector<N, TYPE> A;
		Vector<N, TYPE> B;

		inline Segment() {};
		inline Segment(const Vector<N, TYPE>& p_a, const Vector<N, TYPE>& p_b) { this->A = p_a; this->B = p_b; };
	};

	template<class TYPE>
	struct AABB
	{
		Vector<3, TYPE> center;
		Vector<3, float> radiuses;

		inline AABB() {};
		inline AABB(const Vector<3, TYPE>& p_center, const Vector<3, float>& p_radiuses)
		{
			this->center = p_center;
			this->radiuses = p_radiuses;
		};
	};

	template<class TYPE>
	struct OBB
	{
		Vector<3, TYPE> center;
		Matrix<3, TYPE> rotation;
		Vector<3, float> radiuses;

		inline OBB() {};
		inline OBB(const Vector<3, TYPE>& p_center, const  Matrix<3, TYPE>& p_rotation, const Vector<3, float>& p_radiuses) {
			this->center = p_center;
			this->rotation = p_rotation;
			this->radiuses = p_radiuses;
		};
	};
}
