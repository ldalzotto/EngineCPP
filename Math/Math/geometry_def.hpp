#pragma once

#include "vector_def.hpp"

namespace Math
{
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
		Matrix<4, TYPE> TRS;
		Vector<3, float> radiuses;

		inline OBB() {};
		inline OBB(const Vector<3, TYPE>& p_center, const  Matrix<4, TYPE>& p_TRS, const Vector<3, float>& p_radiuses) {
			this->center = p_center;
			this->TRS = p_TRS;
			this->radiuses = p_radiuses;
		};
	};
}
