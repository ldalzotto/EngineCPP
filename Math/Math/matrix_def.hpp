#pragma once

#include "vector_def.hpp"

namespace Math
{
	template <unsigned N, class TYPE>
	struct Matrix
	{
	};


	template <class TYPE>
	struct alignas(sizeof(TYPE)) Matrix<3, TYPE>
	{
		union
		{
			TYPE Points[9];
			Vector<3, TYPE> Points2D[3];
			struct { TYPE _00, _01, _02, _10, _11, _12, _20, _21, _22; };
			struct { Vector<3, TYPE> Col0, Col1, Col2; };
			struct { Vector<3, TYPE> Right, Up, Forward; };
		};

		Matrix() = default;
		inline Matrix(TYPE p_00, TYPE p_01, TYPE p_02, TYPE p_10, TYPE p_11, TYPE p_12,
			TYPE p_20, TYPE p_21, TYPE p_22 )
			: _00{ p_00 }, _01{ p_01 }, _02{ p_02 }, _10{ p_10 }, _11{ p_11 }, _12{ p_12 },
			_20{ p_20 }, _21{ p_21 }, _22{ p_22 } {}
		inline Matrix(const Vector<3, TYPE>& p_col0, const Vector<3, TYPE>& p_col1, const Vector<3, TYPE>& p_col2) : Col0{ p_col0 }, Col1{ p_col1 }, Col2{ p_col2 } {}

		inline Vector<3, TYPE>& operator[](int p_index)
		{
			return this->Points2D[p_index];
		}
	};

	template <class TYPE>
	struct alignas(sizeof(TYPE)) Matrix<4, TYPE>
	{
		union
		{
			TYPE Points[16];
			Vector<4, TYPE> Points2D[4];
			struct { TYPE _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33; };
			struct { Vector<4, TYPE> Col0, Col1, Col2, Col3; };
			struct { Vector<4, TYPE> Right, Up, Forward, Col3_Direction; };
		};

		Matrix() = default;

		inline Matrix(TYPE p_00, TYPE p_01, TYPE p_02, TYPE p_03, TYPE p_10, TYPE p_11, TYPE p_12,
			TYPE p_13, TYPE p_20, TYPE p_21, TYPE p_22, TYPE p_23, TYPE p_30, TYPE p_31, TYPE p_32, TYPE p_33)
			: _00{ p_00 }, _01{ p_01 }, _02{ p_02 }, _03{ p_03 }, _10{ p_10 }, _11{ p_11 }, _12{ p_12 },
			_13{ p_13 }, _20{ p_20 }, _21{ p_21 }, _22{ p_22 }, _23{ p_23 }, _30{ p_30 }, _31{ p_31 }, _32{ p_32 }, _33{ p_33 } {}

		inline Matrix(const Vector<4, TYPE>& p_col0, const Vector<4, TYPE>& p_col1, const Vector<4, TYPE>& p_col2, const Vector<4, TYPE>& p_col3) : Col0{ p_col0 }, Col1{ p_col1 }, Col2{ p_col2 }, Col3{ p_col3 } {}

		inline Vector<4, TYPE>& operator[](int p_index)
		{
			return this->Points2D[p_index];
		}
	};

	using mat4f = Matrix<4, float>;
	using mat3f = Matrix<3, float>;

	const mat4f mat4f_IDENTITYF = { 1.0f, 0.0f, 0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f };

}