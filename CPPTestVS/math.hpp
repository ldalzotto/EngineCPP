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
		return Vector3<TYPE>(p_left.Points[0] * p_right.Points[0], p_left.Points[1] * p_right.Points[1], p_left.Points[2] * p_right.Points[2]);
	}

	template <class TYPE>
	inline Vector<3, TYPE> mul(const Vector<3, TYPE>& p_left, const TYPE& p_right)
	{
		return Vector3<TYPE>(p_left.Points[0] * p_right, p_left.Points[1] * p_right, p_left.Points[2] * p_right);
	}

	template <class TYPE>
	inline Vector<3, TYPE> min(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector3<TYPE>(p_left.Points[0] - p_right.Points[0], p_left.Points[1] - p_right.Points[1], p_left.Points[2] - p_right.Points[2]);
	}

	template <class TYPE>
	inline Vector<3, TYPE> add(const Vector<3, TYPE>& p_left, const Vector<3, TYPE>& p_right)
	{
		return Vector3<TYPE>(p_left.Points[0] + p_right.Points[0], p_left.Points[1] + p_right.Points[1], p_left.Points[2] + p_right.Points[2]);
	}

	template <class TYPE>
	inline Vector<4, TYPE> mul(const Vector<4, TYPE>& p_left, const Vector<4, TYPE>& p_right)
	{
		return Vector4<TYPE>(p_left.Points[0] * p_right.Points[0], p_left.Points[1] * p_right.Points[1], p_left.Points[2] * p_right.Points[2], p_left.Points[3] * p_right.Points[3]);
	}

	template <class TYPE>
	inline Vector<4, TYPE> mul(const Vector<4, TYPE>& p_left, const TYPE& p_right)
	{
		return Vector4<TYPE>(p_left.Points[0] * p_right, p_left.Points[1] * p_right, p_left.Points[2] * p_right, p_left.Points[3] * p_right);
	}

	template <class TYPE>
	inline Vector<4, TYPE> min(const Vector<4, TYPE>& p_left, const Vector<4, TYPE>& p_right)
	{
		return Vector4<TYPE>(p_left.Points[0] - p_right.Points[0], p_left.Points[1] - p_right.Points[1], p_left.Points[2] - p_right.Points[2], p_left.Points[3] - p_right.Points[3]);
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
		return Vector3<TYPE>(
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
					add(mul(p_left.Vec, p_right.w),
						mul(p_right.Vec, p_left.w)),
					cross(p_left.Vec, p_right.Vec)
				),
				(p_left.w * p_right.w) - dot(p_left.Vec, p_right.Vec)
			)
		);
	}

	/*
	inline Quaternion cross(const Quaternion& p_left, const Quaternion& p_right)
	{

	}
	*/

	inline Quaternion conjugate(const Quaternion& p_left)
	{
		return Quaternion(
			mul(p_left.Vec, -1.0f),
			p_left.Scal
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
		return normalize(mul(l_rotatedVector, conjugate(p_rotation)).Vec);
	};

	inline Quaternion cross(const Quaternion& p_left, const  Quaternion& p_right)
	{
		Vector<3, float> l_rotatedLeft = rotate(vec3f_FORWARD, p_left);
		Vector<3, float> l_rotatedRight = rotate(vec3f_FORWARD, p_right);
		return rotateAround(cross(l_rotatedLeft, l_rotatedRight), 0.0f);
	};

	//TODO extract axis

	template <class TYPE>
	inline Quaternion fromDirection(const Vector<3, TYPE>& p_vec)
	{
		float l_angle = angle(vec3f_FORWARD, p_vec);
		return Quaternion(
			mul(p_vec, sinf(l_angle * 0.5f)),
			cosf(l_angle * 0.5f)
		);
	};

	//TODO from axis

	template <class TYPE>
	inline Quaternion fromEulerAngle(const Vector<3, TYPE>& p_eulerAngle)
	{
		Quaternion l_return = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		l_return = mul(l_return, rotateAround(vec3f_UP, p_eulerAngle.Points[1])); //yaw
		l_return = mul(l_return, rotateAround(vec3f_RIGHT, p_eulerAngle.Points[0])); //pitch
		l_return = mul(l_return, rotateAround(vec3f_FORWARD, p_eulerAngle.Points[2])); //roll
		return l_return;
	};

	template <class TYPE>
	inline Quaternion fromTo(const Vector<3, TYPE>& p_from, const  Vector<3, TYPE>& p_to)
	{
		return rotateAround(cross(p_from, p_to), angle(p_from, p_to));
	};
}


namespace Math
{
	template <unsigned N, class TYPE>
	inline Matrix<N, TYPE> mul(const Matrix<N, TYPE>& p_left, const Matrix<N, TYPE>& p_right)
	{
		//TODO
	}
}