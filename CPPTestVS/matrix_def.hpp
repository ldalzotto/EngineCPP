#pragma once

#include "vector_def.hpp"

namespace Math
{
	template <unsigned N, class TYPE>
	struct Matrix
	{
		float Points[N * N];
	};
	/*
	template <class TYPE>
	struct Matrix<3, TYPE> : public Matrix<TYPE>
	{
		inline Vector<3, TYPE>& Col0()
		{
			return this->Points[0];
		}

		inline Vector<3, TYPE>& Col1()
		{
			return this->Points[3];
		}

		inline Vector<3, TYPE>& Col2()
		{
			return this->Points[6];
		}
	};

	template <class TYPE>
	struct Matrix<4, TYPE> : public Matrix<TYPE>
	{
		inline Vector<4, TYPE>& Col0()
		{
			return this->Points[0];
		}

		inline Vector<4, TYPE>& Col1()
		{
			return this->Points[4];
		}

		inline Vector<4, TYPE>& Col2()
		{
			return this->Points[8];
		}

		inline Vector<4, TYPE>& Col2()
		{
			return this->Points[12];
		}
	};
	*/
}