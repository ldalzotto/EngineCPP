#pragma once

#include "contants.hpp"
#include "functional.hpp"
#include "vector_def.hpp"
#include "quaternion_def.hpp"
#include "matrix_def.hpp"

namespace Math
{
	inline float sqrt(float p_value)
	{
		return sqrtf(p_value);
	}
	inline double sqrt(double p_value)
	{
		return sqrt(p_value);
	}
	inline long double sqrt(long double p_value)
	{
		return sqrtl(p_value);
	}

	template <typename TYPE>
	struct TemplateVector_ForEach_Add
	{
		inline static TYPE execute(const TYPE& p_left, const TYPE& p_right)
		{
			return p_left + p_right;
		}
	};

	template <typename TYPE>
	struct TemplateVector_ForEach_SquareAdd
	{
		inline static TYPE execute(const TYPE& p_left, const TYPE& p_right)
		{
			return p_left + (p_right * p_right);
		}
	};

	template <typename TYPE>
	struct TemplateVector_ForEach_Mul
	{
		inline static TYPE execute(const TYPE& p_left, const TYPE& p_right)
		{
			return p_left * p_right;
		}
	};
}

namespace Math
{
	template <unsigned N, typename TYPE, class Operation>
	struct TemplateVector_Reduce
	{
		inline static TYPE execute(const Vector<N, TYPE>& p, const TYPE& p_start);
	};

	template <typename TYPE, class Operation>
	struct TemplateVector_Reduce<3, TYPE, Operation>
	{
		inline static TYPE execute(const Vector<3, TYPE>& p, const TYPE& p_start)
		{
			return Operation::execute(Operation::execute(Operation::execute(p_start, p.Points[0]), p.Points[1]), p.Points[2]);
		}
	};

	template <typename TYPE, class Operation>
	struct TemplateVector_Reduce<4, TYPE, Operation>
	{
		inline static TYPE execute(const Vector<4, TYPE>& p, const TYPE& p_start)
		{
			return Operation::execute(Operation::execute(Operation::execute(Operation::execute(p_start, p.Points[0]), p.Points[1]), p.Points[2]), p.Points[3]);
		}
	};

	template <unsigned N, typename TYPE, class Operation, class Operation_P1>
	struct TemplateVector_ForEeach
	{
		inline static Vector<N, TYPE> execute(const Vector<N, TYPE>& p, const Operation_P1& p_p1);
	};

	template <typename TYPE, class Operation, class Operation_P1>
	struct TemplateVector_ForEeach<3, TYPE, Operation, Operation_P1>
	{
		inline static Vector<3, TYPE> execute(const Vector<3, TYPE>& p, const Operation_P1& p_p1)
		{
			Vector<3, TYPE> l_return;
			l_return.Points[0] = Operation::execute(p_p1, p.Points[0]);
			l_return.Points[1] = Operation::execute(p_p1, p.Points[1]);
			l_return.Points[2] = Operation::execute(p_p1, p.Points[2]);
			return l_return;
		}
	};

	template <typename TYPE, class Operation, class Operation_P1>
	struct TemplateVector_ForEeach<4, TYPE, Operation, Operation_P1>
	{
		inline static Vector<3, TYPE> execute(const Vector<3, TYPE>& p, const Operation_P1& p_p1)
		{
			Vector<3, TYPE> l_return;
			l_return.Points[0] = Operation::execute(p_p1, p.Points[0]);
			l_return.Points[1] = Operation::execute(p_p1, p.Points[1]);
			l_return.Points[2] = Operation::execute(p_p1, p.Points[2]);
			l_return.Points[3] = Operation::execute(p_p1, p.Points[3]);
			return l_return;
		}
	};

	template <class TYPE>
	inline bool EqualsVec(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Equals<TYPE>(p_left.Points[0], p_right.Points[0])
			&& Equals<TYPE>(p_left.Points[1], p_right.Points[1])
			&& Equals<TYPE>(p_left.Points[2], p_right.Points[2]);
	}

	template <class TYPE>
	inline bool EqualsVec(const Vector<4, TYPE>& p_left, const Vector<4, TYPE>& p_right)
	{
		return Equals<TYPE>(p_left.Points[0], p_right.Points[0])
			&& Equals<TYPE>(p_left.Points[1], p_right.Points[1])
			&& Equals<TYPE>(p_left.Points[2], p_right.Points[2])
			&& Equals<TYPE>(p_left.Points[3], p_right.Points[3]);
	}

	template <class TYPE>
	inline Vector<3, TYPE> mul(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector<3, TYPE>(p_left.Points[0] * p_right.Points[0], p_left.Points[1] * p_right.Points[1], p_left.Points[2] * p_right.Points[2]);
	}

	template <class TYPE>
	inline Vector<3, TYPE> mul(const Vector<3, TYPE>& p_left, const TYPE& p_right)
	{
		return Vector<3, TYPE>(p_left.Points[0] * p_right, p_left.Points[1] * p_right, p_left.Points[2] * p_right);
	}

	template <class TYPE>
	inline Vector<3, TYPE> min(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector<3, TYPE>(p_left.Points[0] - p_right.Points[0], p_left.Points[1] - p_right.Points[1], p_left.Points[2] - p_right.Points[2]);
	}

	template <class TYPE>
	inline Vector<3, TYPE> add(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector<3, TYPE>(p_left.Points[0] + p_right.Points[0], p_left.Points[1] + p_right.Points[1], p_left.Points[2] + p_right.Points[2]);
	}


	template<class TYPE>
	inline Vector<3, TYPE> inv(const Vector<3, TYPE>& p_vec)
	{
		return Vector<3, TYPE>(1.0f / p_vec.Points[0], 1.0f / p_vec.Points[1], 1.0f / p_vec.Points[2]);
	}

	template <class TYPE>
	inline Vector<4, TYPE> mul(const Vector<4, TYPE>& p_left, const Vector<4, TYPE>& p_right)
	{
		return Vector<4, TYPE>(p_left.Points[0] * p_right.Points[0], p_left.Points[1] * p_right.Points[1], p_left.Points[2] * p_right.Points[2], p_left.Points[3] * p_right.Points[3]);
	}

	template <class TYPE>
	inline Vector<4, TYPE> mul(const Vector<4, TYPE>& p_left, const TYPE& p_right)
	{
		return Vector<4, TYPE>(p_left.Points[0] * p_right, p_left.Points[1] * p_right, p_left.Points[2] * p_right, p_left.Points[3] * p_right);
	}

	template <class TYPE>
	inline Vector<4, TYPE> min(const Vector<4, TYPE>& p_left, const Vector<4, TYPE>& p_right)
	{
		return Vector<4, TYPE>(p_left.Points[0] - p_right.Points[0], p_left.Points[1] - p_right.Points[1], p_left.Points[2] - p_right.Points[2], p_left.Points[3] - p_right.Points[3]);
	}

	template<class TYPE>
	inline Vector<4, TYPE> inv(const Vector<4, TYPE>& p_vec)
	{
		return Vector<4, TYPE>(1.0f / p_vec.Points[0], 1.0f / p_vec.Points[1], 1.0f / p_vec.Points[2], 1.0f / p_vec.Points[3]);
	}

	template <unsigned N, class TYPE>
	inline TYPE reduce_sum(const Vector<N, TYPE>& p_left)
	{
		return TemplateVector_Reduce<N, TYPE, TemplateVector_ForEach_Add<TYPE>>::execute(p_left, Zero<TYPE>::zer);
	}

	template <unsigned N, class TYPE>
	inline TYPE dot(const Vector<N, TYPE>& p_left, const Vector<N, TYPE>& p_right)
	{
		return TemplateVector_Reduce<N, TYPE, TemplateVector_ForEach_Add<TYPE>>::execute(mul(p_left, p_right), Zero<TYPE>::zer);
	}

	template <class TYPE>
	inline Vector<3, TYPE> cross(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector<3, TYPE>(
			(p_left.Points[1] * p_right.Points[2]) - (p_left.Points[2] * p_right.Points[1]),
			(p_left.Points[2] * p_right.Points[0]) - (p_left.Points[0] * p_right.Points[2]),
			(p_left.Points[0] * p_right.Points[1]) - (p_left.Points[1] * p_right.Points[0])
			);
	}


	template <unsigned N, class TYPE>
	inline float length(const Vector<N, TYPE>& p_vec)
	{
		return sqrt(TemplateVector_Reduce<N, TYPE, TemplateVector_ForEach_SquareAdd<TYPE>>::execute(p_vec, Zero<TYPE>::zer));
	}

	template <unsigned N, class TYPE>
	inline Vector<N, TYPE> normalize(const Vector<N, TYPE>& p_vec)
	{
		float l_invertedLength = 1.0f / length(p_vec);
		return mul(p_vec, l_invertedLength);
	}

	template <class TYPE>
	inline Vector<3, TYPE> project(const Vector<3, TYPE>& p_vec, const Vector<3, TYPE>& p_projectedOn)
	{
		return mul(p_projectedOn, (dot(p_vec, p_projectedOn) / length(p_projectedOn)));
	}

	template <unsigned N, class TYPE>
	inline float distance(const Vector<N, TYPE>& p_start, const Vector<N, TYPE>& p_end)
	{
		return length(min(p_start, p_end));
	}

	template <class TYPE>
	inline float angle(const Vector<3, TYPE>& p_begin, const Vector<3, TYPE>& p_end)
	{
		return acosf(
			dot(p_begin, p_end) / (length(p_begin) * length(p_end))
		);
	};

	template <class TYPE>
	inline float angle_normalized(const Vector<3, TYPE>& p_begin, const Vector<3, TYPE>& p_end)
	{
		return acosf(dot(p_begin, p_end));
	};

	template <class TYPE>
	short int anglesign(const Vector<3, TYPE>& p_begin, const Vector<3, TYPE>& p_end, const Vector<3, TYPE>& p_referenceAxis)
	{
		Vector<3, TYPE> l_cross = cross(p_begin, p_end);
		float l_dot = dot(l_cross, p_referenceAxis);
		return l_dot >= Tolerance<float>::tol ? 1 : -1;
	};

}


namespace Math
{
	inline bool Equals(const Quaternion& p_left, const Quaternion& p_right)
	{
		return EqualsVec(p_left.Points, p_right.Points);
	}

	inline Quaternion normalize(const Quaternion& p_quat)
	{
		return Quaternion(mul(p_quat.Points, 1.0f / length(p_quat.Points)));
	}

	inline Quaternion mul(const Quaternion& p_left, const Quaternion& p_right)
	{
		return normalize(
			Quaternion(
				add(
					add(mul(p_left.Vec3s.Vec, p_right.w),
						mul(p_right.Vec3s.Vec, p_left.w)),
					cross(p_left.Vec3s.Vec, p_right.Vec3s.Vec)
				),
				(p_left.w * p_right.w) - dot(p_left.Vec3s.Vec, p_right.Vec3s.Vec)
			)
		);
	}

	inline Quaternion inv(const Quaternion& p_left)
	{
		return Quaternion(
			mul(p_left.Vec3s.Vec, -1.0f),
			p_left.Vec3s.Scal
		);
	};

	template <class TYPE>
	inline Quaternion rotateAround(const Vector<3, TYPE> p_axis, const  float p_angle)
	{
		return Quaternion(
			mul(p_axis, sinf(p_angle * 0.5f)),
			cosf(p_angle * 0.5f)
		);
	};

	template <class TYPE>
	inline Vector<3, TYPE> rotate(const Vector<3, TYPE>& p_vector, const Quaternion& p_rotation)
	{
		Quaternion l_vectorAsQuat(p_vector, 0.0f);
		Quaternion l_rotatedVector = mul(p_rotation, l_vectorAsQuat);
		return mul(mul(l_rotatedVector, inv(p_rotation)).Vec3s.Vec, length(p_vector));
	};

	inline Quaternion cross(const Quaternion& p_left, const  Quaternion& p_right)
	{
		Vector<3, float> l_rotatedLeft = normalize(rotate(VecConst<float>::FORWARD, p_left));
		Vector<3, float> l_rotatedRight = normalize(rotate(VecConst<float>::FORWARD, p_right));
		return rotateAround(cross(l_rotatedLeft, l_rotatedRight), 0.0f);
	};


	template <class TYPE>
	inline Matrix<3, TYPE> extractAxis(const Quaternion& quat)
	{
		float l_qxx = quat.x * quat.x;
		float l_qxy = quat.x * quat.y;
		float l_qxz = quat.x * quat.z;
		float l_qxw = quat.x * quat.w;

		float l_qyy = quat.y * quat.y;
		float l_qyz = quat.y * quat.z;
		float l_qyw = quat.y * quat.w;

		float l_qzz = quat.z * quat.z;
		float l_qzw = quat.z * quat.w;

		Matrix<3, TYPE> l_return;
		//RIGHT
		l_return.Col0 = Vector<3, TYPE>(
			1 - (2 * l_qyy) - (2 * l_qzz),
			(2 * l_qxy) + (2 * l_qzw),
			(2 * l_qxz) - (2 * l_qyw)
			);

		//UP	
		l_return.Col1 = Vector<3, TYPE>(
			(2 * l_qxy) - (2 * l_qzw),
			1 - (2 * l_qxx) - (2 * l_qzz),
			(2 * l_qyz) + (2 * l_qxw)
			);

		//Forward
		l_return.Col2 = Vector<3, TYPE>(
			(2 * l_qxz) + (2 * l_qyw),
			(2 * l_qyz) - (2 * l_qxw),
			1 - (2 * l_qxx) - (2 * l_qyy)
			);

		l_return.Col0 = normalize(l_return.Col0);
		l_return.Col1 = normalize(l_return.Col1);
		l_return.Col2 = normalize(l_return.Col2);

		return l_return;
	};

	inline Quaternion fromAxis(const Matrix<3, float>& p_axis)
	{
		const Vector<3, float>& l_right = p_axis.Right;
		const Vector<3, float>& l_up = p_axis.Up;
		const Vector<3, float>& l_forward = p_axis.Forward;

		// We calculate the four square roots and get the higher one.
		float qxDiag = fmaxf(1 + l_right.x - l_up.y - l_forward.z, 0.0f);
		float qyDiag = fmaxf(1 + l_up.y - l_right.x - l_forward.z, 0.0f);
		float qzDiag = fmaxf(1 + l_forward.z - l_right.x - l_up.y, 0.0f);
		float qwDiag = fmaxf(1 + l_right.x + l_up.y + l_forward.z, 0.0f);

		int l_diagonalIndex = 0;
		float l_biggestDiagonalValue = qxDiag;
		if (qyDiag > l_biggestDiagonalValue)
		{
			l_biggestDiagonalValue = qyDiag;
			l_diagonalIndex = 1;
		}
		if (qzDiag > l_biggestDiagonalValue)
		{
			l_biggestDiagonalValue = qzDiag;
			l_diagonalIndex = 2;
		}
		if (qwDiag > l_biggestDiagonalValue)
		{
			l_biggestDiagonalValue = qwDiag;
			l_diagonalIndex = 3;
		}

		l_biggestDiagonalValue = 0.5f * sqrtf(l_biggestDiagonalValue);
		float mult = 1 / (4.0f * l_biggestDiagonalValue);

		switch (l_diagonalIndex)
		{
		case 0:
		{
			return Quaternion(
				l_biggestDiagonalValue,
				(l_right.y + l_up.x) * mult,
				(l_forward.x + l_right.z) * mult,
				(l_up.z - l_forward.y) * mult
			);
		}
		break;
		case 1:
		{
			return Quaternion(
				(l_right.y + l_up.x) * mult,
				l_biggestDiagonalValue,
				(l_up.z + l_forward.y) * mult,
				(l_forward.x - l_right.z) * mult
			);
		}
		break;
		case 2:
		{
			return Quaternion(
				(l_forward.x + l_right.z) * mult,
				(l_up.z + l_forward.y) * mult,
				l_biggestDiagonalValue,
				(l_right.y - l_up.x) * mult
			);
		}
		break;
		case 3:
		{
			return Quaternion(
				(l_up.z - l_forward.y) * mult,
				(l_forward.x - l_right.z) * mult,
				(l_right.y - l_up.x) * mult,
				l_biggestDiagonalValue
			);
		}
		break;
		}

		return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	};

	template <class TYPE>
	inline Quaternion fromEulerAngle(const Vector<3, TYPE>& p_eulerAngle)
	{
		Quaternion l_return;

		Vector<3, TYPE> l_cos = Vector<3, TYPE>(cos(p_eulerAngle.Points[0] * (TYPE)0.5), cos(p_eulerAngle.Points[1] * (TYPE)0.5), cos(p_eulerAngle.Points[2] * (TYPE)0.5));
		Vector<3, TYPE> l_sin = Vector<3, TYPE>(sin(p_eulerAngle.Points[0] * (TYPE)0.5), sin(p_eulerAngle.Points[1] * (TYPE)0.5), sin(p_eulerAngle.Points[2] * (TYPE)0.5));

		l_return.x = l_sin.x * l_cos.y * l_cos.z - l_cos.x * l_sin.y * l_sin.z;
		l_return.y = l_cos.x * l_sin.y * l_cos.z + l_sin.x * l_cos.y * l_sin.z;
		l_return.z = l_cos.x * l_cos.y * l_sin.z - l_sin.x * l_sin.y * l_cos.z;
		l_return.w = l_cos.x * l_cos.y * l_cos.z + l_sin.x * l_sin.y * l_sin.z;

		return l_return;
	};

	template<class TYPE>
	inline Vector<3, TYPE> eulerAngle(const Quaternion& p_quat)
	{
		Vector<3, TYPE> l_return;

		//pitch
		float l_sinp = ((TYPE)2) * (p_quat.y * p_quat.z + p_quat.w * p_quat.x);
		float l_cosp = p_quat.w * p_quat.w - p_quat.x * p_quat.x - p_quat.y * p_quat.y + p_quat.z * p_quat.z;

		if (Math::Equals<TYPE>(l_sinp, Zero<TYPE>::zer) && Math::Equals<TYPE>(l_cosp, Zero<TYPE>::zer))
		{
			l_return.Points[0] = ((TYPE)2) * atan2(p_quat.x, p_quat.w);
		}
		else
		{
			l_return.Points[0] = atan2(l_sinp, l_cosp);
		}

		//yaw
		l_return.Points[1] =
			asin(Math::clamp(((TYPE)-2) * (p_quat.x * p_quat.z - p_quat.w * p_quat.y), (TYPE)-1, (TYPE)1));

		//roll
		l_return.Points[2] =
			atan2(((TYPE)2) * (p_quat.x * p_quat.y + p_quat.w * p_quat.z), p_quat.w * p_quat.w + p_quat.x * p_quat.x - p_quat.y * p_quat.y - p_quat.z * p_quat.z)
			;


		return l_return;
	}

	template <class TYPE>
	inline Quaternion fromTo(const Vector<3, TYPE>& p_from, const  Vector<3, TYPE>& p_to)
	{
		return fromTo_normalized(normalize(p_from), normalize(p_to));
	};

	template <class TYPE>
	inline Quaternion fromTo_normalized(const Vector<3, TYPE>& p_from, const  Vector<3, TYPE>& p_to)
	{
		TYPE l_costtheta = dot(p_from, p_to);
		if (l_costtheta >= One<TYPE>::one - Tolerance<TYPE>::tol)
		{
			return QuatConst::IDENTITY;
		}

		Vector<3, TYPE> l_rotation_axis;

		if (l_costtheta < -One<TYPE>::one + Tolerance<TYPE>::tol)
		{
			l_rotation_axis = cross(VecConst<TYPE>::FORWARD, p_from);
			if (length(l_rotation_axis) < Tolerance<TYPE>::tol)
			{
				l_rotation_axis = cross(VecConst<TYPE>::RIGHT, p_from);
			}
			l_rotation_axis = normalize(l_rotation_axis);
			return rotateAround(l_rotation_axis, M_PI);
		}

		l_rotation_axis = cross(p_from, p_to);
		TYPE l_s = sqrt((One<TYPE>::one + l_costtheta) * (TYPE)2);
		TYPE l_invs = (TYPE)1 / l_s;

		return Quaternion(
			l_rotation_axis.x * l_invs,
			l_rotation_axis.y * l_invs,
			l_rotation_axis.z * l_invs,
			l_s * (TYPE)0.5f
		);
	};
}


namespace Math
{
	template <unsigned N, class TYPE>
	inline TYPE mul_line_column(const Matrix<N, TYPE>& p_left, const Matrix<N, TYPE>& p_right, const short int p_column_index, const short int p_line_index)
	{
		TYPE l_return = 0;
		for (short int i = 0; i < N; i++)
		{
			l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points2D[p_column_index].Points[i]);
		}
		return l_return;
	};

	template <unsigned N, class TYPE>
	inline TYPE mul_line_vec(const Matrix<N, TYPE>& p_left, const Vector<N, TYPE>& p_right, const short int p_line_index)
	{
		TYPE l_return = Zero<TYPE>::zer;
		for (short int i = 0; i < N; i++)
		{
			l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points[i]);
		}
		return l_return;
	};


	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> mul(const Matrix<N, TYPE>& p_left, const Matrix<N, TYPE>& p_right)
	{
		Matrix<N, TYPE> l_return;
		for (short int p_column_index = 0; p_column_index < N; p_column_index++)
		{
			for (short int p_line_index = 0; p_line_index < N; p_line_index++)
			{
				l_return[p_column_index][p_line_index] = mul_line_column<N, TYPE>(p_left, p_right, p_column_index, p_line_index);
			}
		}
		return l_return;
	};

	template <unsigned N, class TYPE>
	inline Vector<N, TYPE> mul(const Matrix<N, TYPE>& p_left, const Vector<N, TYPE>& p_right)
	{
		Vector<N, TYPE> l_return;
		for (short int p_line_index = 0; p_line_index < N; p_line_index++)
		{
			l_return[p_line_index] = mul_line_vec(p_left, p_right, p_line_index);
		}
		return l_return;
	};

	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> mul(const Matrix<N, TYPE>& p_left, const TYPE& p_right)
	{
		Matrix<N, TYPE> l_return;
		for (short int p_column_index = 0; p_column_index < N; p_column_index++)
		{
			for (short int p_line_index = 0; p_line_index < N; p_line_index++)
			{
				l_return[p_column_index][p_line_index] = p_left.Points2D[p_column_index].Points[p_line_index] * p_right;
			}
		}
		return l_return;
	};

	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> add(const Matrix<N, TYPE>& p_left, const TYPE& p_right)
	{
		Matrix<N, TYPE> l_return;
		for (short int p_column_index = 0; p_column_index < N; p_column_index++)
		{
			for (short int p_line_index = 0; p_line_index < N; p_line_index++)
			{
				l_return[p_column_index][p_line_index] = p_left.Points2D[p_column_index].Points[p_line_index] + p_right;
			}
		}
		return l_return;
	};

	template <class TYPE>
	inline TYPE det(const Matrix<4, TYPE>& p_mat, const short int p_column_index, const short int p_line_index)
	{
		Matrix<3, TYPE> l_matDet;
		short int l_matDet_column_counter = 0;
		short int l_matDet_line_counter = 0;

		for (short int l_column_index = 0; l_column_index < 4; l_column_index++)
		{
			if (l_column_index != p_column_index)
			{
				l_matDet_line_counter = 0;
				for (short int l_line_index = 0; l_line_index < 4; l_line_index++)
				{
					if (l_line_index != p_line_index)
					{
						l_matDet.Points2D[l_matDet_column_counter].Points[l_matDet_line_counter] = p_mat.Points2D[l_column_index].Points[l_line_index];
						l_matDet_line_counter += 1;
					}

				}
				l_matDet_column_counter += 1;
			}

		}

		return
			(l_matDet[0][0] * ((l_matDet[1][1] * l_matDet[2][2]) - (l_matDet[1][2] * l_matDet[2][1]))) +
			(l_matDet[1][0] * ((l_matDet[2][1] * l_matDet[0][2]) - (l_matDet[2][2] * l_matDet[0][1]))) +
			(l_matDet[2][0] * ((l_matDet[0][1] * l_matDet[1][2]) - (l_matDet[0][2] * l_matDet[1][1])));
	}

	template <class TYPE>
	inline Matrix<4, TYPE> inv(const Matrix<4, TYPE>& p_mat)
	{
		Matrix<4, TYPE> l_return;
		float l_det =
			(p_mat.Points2D[0].Points[0] * det(p_mat, 0, 0))
			- (p_mat.Points2D[0].Points[1] * det(p_mat, 0, 1))
			+ (p_mat.Points2D[0].Points[2] * det(p_mat, 0, 2))
			- (p_mat.Points2D[0].Points[3] * det(p_mat, 0, 3));

		{
			l_return._00 = det(p_mat, 0, 0);
			l_return._01 = -det(p_mat, 1, 0);
			l_return._02 = det(p_mat, 2, 0);
			l_return._03 = -det(p_mat, 3, 0);
			l_return._10 = -det(p_mat, 0, 1);
			l_return._11 = det(p_mat, 1, 1);
			l_return._12 = -det(p_mat, 2, 1);
			l_return._13 = det(p_mat, 3, 1);
			l_return._20 = det(p_mat, 0, 2);
			l_return._21 = -det(p_mat, 1, 2);
			l_return._22 = det(p_mat, 2, 2);
			l_return._23 = -det(p_mat, 3, 2);
			l_return._30 = -det(p_mat, 0, 3);
			l_return._31 = det(p_mat, 1, 3);
			l_return._32 = -det(p_mat, 2, 3);
			l_return._33 = det(p_mat, 3, 3);
		}

		return mul(l_return, 1.0f / l_det);
	}

	template <class TYPE>
	inline Matrix<4, TYPE> translationMatrix(const Vector<3, TYPE>& p_translation)
	{
		Matrix<4, TYPE> l_return = mat4f_IDENTITYF;
		l_return.Col3.Vec3 = p_translation;
		return l_return;
	}

	template<class TYPE>
	inline Vector<3, TYPE> translationVector(const Matrix<4, TYPE>& p_trs)
	{
		return p_trs.Col3.Vec3;
	}

	template <class TYPE>
	inline Matrix<4, TYPE> rotationMatrix(const Matrix<3, TYPE>& p_axis)
	{
		return Matrix<4, TYPE>(
			Vector<4, TYPE>(p_axis.Points2D[0], 0.0f),
			Vector<4, TYPE>(p_axis.Points2D[1], 0.0f),
			Vector<4, TYPE>(p_axis.Points2D[2], 0.0f),
			Vector<4, TYPE>(0.0f, 0.0f, 0.0f, 1.0f)
			);
	}

	template <unsigned N, class TYPE>
	struct RotationMatrixKernel
	{
		static Matrix<N, TYPE> rotationMatrix(const Vector<3, TYPE>& p_right, const Vector<3, TYPE>& p_up, const Vector<3, TYPE>& p_forward);
	};

	template <class TYPE>
	struct RotationMatrixKernel<4, TYPE>
	{
		inline static Matrix<4, TYPE> rotationMatrix(const Vector<3, TYPE>& p_right, const Vector<3, TYPE>& p_up, const Vector<3, TYPE>& p_forward)
		{
			return Matrix<4, TYPE>(
				Vector<4, TYPE>(p_right, 0.0f),
				Vector<4, TYPE>(p_up, 0.0f),
				Vector<4, TYPE>(p_forward, 0.0f),
				Vector<4, TYPE>(0.0f, 0.0f, 0.0f, 1.0f)
				);
		};
	};

	template <class TYPE>
	struct RotationMatrixKernel<3, TYPE>
	{
		inline static Matrix<3, TYPE> rotationMatrix(const Vector<3, TYPE>& p_right, const Vector<3, TYPE>& p_up, const Vector<3, TYPE>& p_forward)
		{
			return Matrix<3, TYPE>(p_right, p_up, p_forward);
		};
	};

	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> rotationMatrix(const Vector<3, TYPE>& p_right, const Vector<3, TYPE>& p_up, const Vector<3, TYPE>& p_forward)
	{
		return RotationMatrixKernel<N, TYPE>::rotationMatrix(p_right, p_up, p_forward);
	};


	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> lookAtRotation(const Vector<3, TYPE>& p_origin, const Vector<3, TYPE>& p_target, const Vector<3, TYPE>& p_up)
	{
		Vector<3, TYPE> l_forward = normalize(min(p_target, p_origin));
		Vector<3, TYPE> l_right = normalize(cross(l_forward, p_up));
		Vector<3, TYPE> l_up = normalize(cross(l_right, l_forward));

		return rotationMatrix<N, TYPE>(
			l_right,
			l_up,
			l_forward
			);
	};

	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> lookAtRotation_viewmatrix(const Vector<3, TYPE>& p_origin, const Vector<3, TYPE>& p_target, const Vector<3, TYPE>& p_up)
	{
		Vector<3, TYPE> l_forward = normalize(min(p_target, p_origin));
		Vector<3, TYPE> l_right = normalize(mul(cross(l_forward, p_up), -One<float>::one));
		Vector<3, TYPE> l_up = normalize(mul(cross(l_right, l_forward), -One<float>::one));

		return rotationMatrix<N, TYPE>(
			l_right,
			l_up,
			l_forward
			);
	}

	template <class TYPE>
	inline Matrix<4, TYPE> scaleMatrix(const Vector<3, TYPE>& p_scale)
	{
		Matrix<4, TYPE> l_return = mat4f_IDENTITYF;
		l_return.Col0 = mul(l_return.Col0, p_scale.Points[0]);
		l_return.Col1 = mul(l_return.Col1, p_scale.Points[1]);
		l_return.Col2 = mul(l_return.Col2, p_scale.Points[2]);
		return l_return;
	}

	template <class TYPE>
	inline Matrix<4, TYPE> TRS(const Vector<3, TYPE>& p_position, const Matrix<3, TYPE>& p_axis, const Vector<3, TYPE>& p_scale)
	{
		return
			mul(
				mul(
					translationMatrix(p_position),
					rotationMatrix(p_axis)),
				scaleMatrix(p_scale)
			);
	}

	template<class TYPE>
	inline Matrix<4, TYPE> TRS_getlocal_from(const Matrix<4, TYPE>& p_from, const Matrix<4, TYPE>& p_to)
	{
		return mul(p_from, inv(p_to));
	};

	template <class TYPE>
	inline Matrix<4, TYPE> perspective(const float p_fov, const float p_aspect, const float p_near, const float p_far)
	{
		Matrix<4, TYPE> l_return;
		TYPE l_halfTan = tan(p_fov / 2.0f);

		l_return._00 = 1.0f / (p_aspect * l_halfTan);
		l_return._01 = 0.0f;
		l_return._02 = 0.0f;
		l_return._03 = 0.0f;

		l_return._10 = 0.0f;
		l_return._11 = 1.0f / l_halfTan;
		l_return._12 = 0.0f;
		l_return._13 = 0.0f;

		l_return._20 = 0.0f;
		l_return._21 = 0.0f;
		l_return._22 = (p_far + p_near) / (p_far - p_near);
		l_return._23 = 1.0f;

		l_return._30 = 0.0f;
		l_return._31 = 0.0f;
		l_return._32 = (-2.0f * p_far * p_near) / (p_far - p_near);
		l_return._33 = 0.0f;

		return l_return;
	}

	template<class TYPE>
	inline Matrix<4, TYPE> view(const Vector<3, TYPE>& p_world_position, const Vector<3, TYPE>& p_forward, const Vector<3, TYPE>& p_up)
	{
		Vector<3, TYPE> l_target = p_forward;
		l_target = add(p_world_position, l_target);

		Vector<3, TYPE> l_up = mul(Math::normalize(p_up), -1.0f);
		Matrix<4, TYPE> l_view = TRS(p_world_position, lookAtRotation_viewmatrix<3, TYPE>(p_world_position, l_target, l_up), VecConst<TYPE>::ONE);
		return inv(l_view);
	}



}
