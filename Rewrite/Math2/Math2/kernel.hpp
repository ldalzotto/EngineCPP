#pragma once

namespace v2
{
	inline char Math::equals(const float p_left, const float p_right)
	{
		return fabsf(p_left - p_right) <= Math::tol_f;
	};

	inline char Math::nequals(const float p_left, const float p_right)
	{
		return fabsf(p_left - p_right) > Math::tol_f;
	};

	inline char Math::lower_eq(const float p_left, const float p_right)
	{
		return (p_left - p_right) <= Math::tol_f;
	};

	inline char Math::lower(const float p_left, const float p_right)
	{
		return (p_left - p_right) < Math::tol_f;
	};

	inline char Math::greater_eq(const float p_left, const float p_right)
	{
		return (p_left - p_right) >= Math::tol_f;
	};

	inline char Math::greater(const float p_left, const float p_right)
	{
		return (p_left - p_right) > Math::tol_f;
	};

	inline short int Math::sign(const float p_value)
	{
		if (p_value <= -Math::tol_f)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	};

	inline float Math::clamp(const float p_value, const float p_left, const float p_right)
	{
		if (p_value >= (p_right + Math::tol_f))
		{
			return p_right;
		}
		else if (p_value <= (p_left + Math::tol_f))
		{
			return p_left;
		}
		return p_value;
	};

}


#define math_v2f_foreach_2(Left, Right, Op) \
    v2f { Op((Left)->Points[0], (Right)->Points[0]), Op((Left)->Points[1], (Right)->Points[1]) }

#define math_v3f_foreach(Left, Op) \
    v3f { Op((Left)->Points[0]), Op((Left)->Points[1]), Op((Left)->Points[2]) }

#define math_v3f_foreach_2(Left, Right, Op) \
    v3f { Op((Left)->Points[0], (Right)->Points[0]), Op((Left)->Points[1], (Right)->Points[1]), Op((Left)->Points[2], (Right)->Points[2]) }

#define math_v3f_reduce(Left, Op) \
    Op(Op((Left)->Points[0], (Left)->Points[1]), (Left)->Points[2])

#define math_v4f_foreach(Left, Op) \
    v4f { Op((Left)->Points[0]), Op((Left)->Points[1]), Op((Left)->Points[2]), Op((Left)->Points[3]) }

#define math_v4f_foreach_2(Left, Right, Op) \
    v4f { Op((Left)->Points[0], (Right)->Points[0]), Op((Left)->Points[1], (Right)->Points[1]), Op((Left)->Points[2], (Right)->Points[2]), Op((Left)->Points[3], (Right)->Points[3]) }

#define math_v4f_reduce(Left, Op) \
    Op(Op(Op((Left)->Points[0], (Left)->Points[1]), (Left)->Points[2]), (Left)->Points[3])

#define math_mul_op(Left, Right) Left *Right
#define math_add_op(Left, Right) Left + Right
#define math_min_op(Left, Right) Left - Right
#define math_square_op(Left) Left * Left
#define math_inv_op(Left) 1.0f / Left

inline char v2f::operator==(const v2f& p_other)
{
	return v2::Math::equals(this->Points[0], p_other.Points[0]) &&
		v2::Math::equals(this->Points[1], p_other.Points[1]);
};

inline v2f v2f::operator*(const v2f& p_other)
{
	return math_v2f_foreach_2(this, &p_other, math_mul_op);
};

inline char v3f_assert_is_normalized(const v3f& p_vec)
{
	return v2::Math::equals(p_vec.length(), 1.0f);
};

inline v3f v3f::operator+(const v3f& p_other) const
{
	return math_v3f_foreach_2(this, &p_other, math_add_op);
};

inline v3f v3f::operator*(const float p_other) const
{
	return v3f{ this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other };
};

inline v3f v3f::operator*(const v3f& p_other) const
{
	return math_v3f_foreach_2(this, &p_other, math_mul_op);
};

inline v3f v3f::operator-(const v3f& p_other) const
{
	return math_v3f_foreach_2(this, &p_other, math_min_op);
};

inline char v3f::operator==(const v3f& p_other) const
{
	return v2::Math::equals(this->Points[0], p_other.Points[0]) &&
		v2::Math::equals(this->Points[1], p_other.Points[1]) &&
		v2::Math::equals(this->Points[2], p_other.Points[2]);
};

inline char v3f::operator!=(const v3f& p_other) const
{
	return v2::Math::nequals(this->Points[0], p_other.Points[0]) ||
		v2::Math::nequals(this->Points[1], p_other.Points[1]) ||
		v2::Math::nequals(this->Points[2], p_other.Points[2]);
};

inline float& v3f::operator[](const unsigned char p_index)
{
	return this->Points[p_index];
};

inline float v3f::dot(const v3f& p_other) const
{
	v3f l_mul = this->operator*(p_other);
	return math_v3f_reduce(&l_mul, math_add_op);
};

inline v3f v3f::cross(const v3f& p_other) const
{
	return v3f{
		(this->Points[1] * p_other.Points[2]) - (this->Points[2] * p_other.Points[1]),
		(this->Points[2] * p_other.Points[0]) - (this->Points[0] * p_other.Points[2]),
		(this->Points[0] * p_other.Points[1]) - (this->Points[1] * p_other.Points[0]) };
};

inline float v3f::length() const
{
	v3f l_squared = math_v3f_foreach(this, math_square_op);
	return sqrtf(math_v3f_reduce(&l_squared, math_add_op));
};

inline v3f v3f::normalize() const
{
	return this->operator*(1.0f / this->length());
};

inline v3f v3f::inv() const
{
	return math_v3f_foreach(this, math_inv_op);
};

inline v3f v3f::project(const v3f& p_projected_on) const
{
	return this->normalize().project_normalized(p_projected_on.normalize());
};

inline v3f v3f::project_normalized(const v3f& p_projected_on) const
{
#if MATH_NORMALIZATION_TEST
	assert_true(v3f_assert_is_normalized(*this));
	assert_true(v3f_assert_is_normalized(p_projected_on));
#endif

	return p_projected_on * this->dot(p_projected_on);
};

inline float v3f::distance(const v3f& p_end) const
{
	return this->operator-(p_end).length();
};

inline float v3f::angle_unsigned(const v3f& p_end) const
{
	return acosf(this->dot(p_end) / (this->length() * p_end.length()));
};

inline float v3f::angle_unsigned_normalized(const v3f& p_end_normalized) const
{
#ifdef MATH_NORMALIZATION_TEST
	v3f_assert_is_normalized(*this);
	v3f_assert_is_normalized(p_end_normalized);
#endif

	return acosf(this->dot(p_end_normalized));
};

inline char v3f::anglesign(const v3f& p_end, const v3f& p_ref_axis) const
{
	float l_dot = this->cross(p_end).dot(p_ref_axis);
	return l_dot >= v2::Math::tol_f ? 1 : -1;
};

inline v3f v3f::rotate(const quat& p_rotation) const
{
	return (p_rotation * quat::build_v3f_f(*this, 0.0f) * p_rotation.inv()).Vec3s.Vec * this->length();
};

inline quat v3f::euler_to_quat() const
{
	v3f l_cos = v3f{ cosf(this->Points[0] * 0.5f), cosf(this->Points[1] * 0.5f), cosf(this->Points[2] * 0.5f) };
	v3f l_sin = v3f{ sinf(this->Points[0] * 0.5f), sinf(this->Points[1] * 0.5f), sinf(this->Points[2] * 0.5f) };

	return quat
	{
		l_sin.x * l_cos.y * l_cos.z - l_cos.x * l_sin.y * l_sin.z,
		l_cos.x * l_sin.y * l_cos.z + l_sin.x * l_cos.y * l_sin.z,
		l_cos.x * l_cos.y * l_sin.z - l_sin.x * l_sin.y * l_cos.z,
		l_cos.x * l_cos.y * l_cos.z + l_sin.x * l_sin.y * l_sin.z
	};
};

inline quat v3f::from_to(const v3f& p_to) const
{
	return this->normalize().from_to_normalized(p_to.normalize());
};


inline quat v3f::from_to_normalized(const v3f& p_to) const
{
#ifdef MATH_NORMALIZATION_TEST
	v3f_assert_is_normalized(*this);
	v3f_assert_is_normalized(p_to);
#endif

	float l_costtheta = this->dot(p_to);
	if (l_costtheta >= v2::Math::one_f - v2::Math::tol_f)
	{
		return quat_const::IDENTITY;
	}

	v3f l_rotation_axis;

	if (l_costtheta < -v2::Math::one_f + v2::Math::tol_f)
	{
		l_rotation_axis = v3f_const::FORWARD.cross(*this);
		if (l_rotation_axis.length() < v2::Math::tol_f)
		{
			l_rotation_axis = v3f_const::RIGHT.cross(*this);
		}
		l_rotation_axis = l_rotation_axis.normalize();
		return quat::rotate_around(l_rotation_axis, v2::Math::M_PI);
	}

	l_rotation_axis = this->cross(p_to);
	float l_s = sqrtf((v2::Math::one_f + l_costtheta) * 2.0f);
	float l_invs = 1.0f / l_s;

	return quat{
		l_rotation_axis.x * l_invs,
		l_rotation_axis.y * l_invs,
		l_rotation_axis.z * l_invs,
		l_s * 0.5f
	};
};

inline char v4f::operator==(const v4f& p_other) const
{
	return v2::Math::equals(this->Points[0], p_other.Points[0]) &&
		v2::Math::equals(this->Points[1], p_other.Points[1]) &&
		v2::Math::equals(this->Points[2], p_other.Points[2]) &&
		v2::Math::equals(this->Points[3], p_other.Points[3]);
};

inline char v4f::operator!=(const v4f& p_other) const
{
	return v2::Math::nequals(this->Points[0], p_other.Points[0]) ||
		v2::Math::nequals(this->Points[1], p_other.Points[1]) ||
		v2::Math::nequals(this->Points[2], p_other.Points[2]) ||
		v2::Math::nequals(this->Points[3], p_other.Points[3]);
};

inline v4f v4f::operator*(const float p_other) const
{
	return v4f{ this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other, this->Points[3] * p_other };
};

inline v4f v4f::operator*(const v4f& p_other) const
{
	return math_v4f_foreach_2(this, &p_other, math_mul_op);
};

inline float& v4f::operator[](const unsigned char p_index)
{
	return this->Points[p_index];
};


inline float v4f::length() const
{
	v4f l_squared = math_v4f_foreach(this, math_square_op);
	return sqrtf(math_v4f_reduce(&l_squared, math_add_op));
};

inline quat quat::rotate_around(const v3f& p_axis, const float p_angle)
{
	return quat::build_v3f_f(p_axis * sinf(p_angle * 0.5f), cosf(p_angle * 0.5f));
};

inline char quat::operator==(const quat& p_other) const
{
	return this->Points == p_other.Points;
};

inline char quat::operator!=(const quat& p_other) const
{
	return this->Points != p_other.Points;
};

inline quat quat::operator*(const quat& p_other) const
{
	return quat::build_v3f_f(
		((this->Vec3s.Vec * p_other.w) + (p_other.Vec3s.Vec * this->w)) + this->Vec3s.Vec.cross(p_other.Vec3s.Vec),
		(this->w * p_other.w) - this->Vec3s.Vec.dot(p_other.Vec3s.Vec))
		.normalize();
};

inline quat quat::normalize() const
{
	return quat::build_v4f(this->Points * (1.0f / this->Points.length()));
};

inline quat quat::inv() const
{
	return quat::build_v3f_f(
		this->Vec3s.Vec * -1.0f,
		this->Vec3s.Scal);
};

inline quat quat::cross(const quat& p_other) const
{
	v3f l_rotated_left = v3f_const::FORWARD.rotate(*this).normalize();
	v3f l_rotated_right = v3f_const::FORWARD.rotate(p_other).normalize();
	return quat::rotate_around(l_rotated_left.cross(l_rotated_right), 0.0f);
};

inline m33f quat::to_axis() const
{

	float l_qxx = this->x * this->x;
	float l_qxy = this->x * this->y;
	float l_qxz = this->x * this->z;
	float l_qxw = this->x * this->w;

	float l_qyy = this->y * this->y;
	float l_qyz = this->y * this->z;
	float l_qyw = this->y * this->w;

	float l_qzz = this->z * this->z;
	float l_qzw = this->z * this->w;

	m33f l_return;
	//RIGHT
	l_return.Col0 = v3f{
		1 - (2 * l_qyy) - (2 * l_qzz),
		(2 * l_qxy) + (2 * l_qzw),
		(2 * l_qxz) - (2 * l_qyw) };

	//UP
	l_return.Col1 = v3f{
		(2 * l_qxy) - (2 * l_qzw),
		1 - (2 * l_qxx) - (2 * l_qzz),
		(2 * l_qyz) + (2 * l_qxw) };

	//Forward
	l_return.Col2 = v3f{
		(2 * l_qxz) + (2 * l_qyw),
		(2 * l_qyz) - (2 * l_qxw),
		1 - (2 * l_qxx) - (2 * l_qyy) };

	l_return.Col0 = l_return.Col0.normalize();
	l_return.Col1 = l_return.Col1.normalize();
	l_return.Col2 = l_return.Col2.normalize();

	return l_return;
};

inline v3f quat::euler() const
{
	v3f l_return;

	//pitch
	float l_sinp = 2.0f * (this->y * this->z + this->w * this->x);
	float l_cosp = this->w * this->w - this->x * this->x - this->y * this->y + this->z * this->z;

	if (v2::Math::equals(l_sinp, v2::Math::zero_f) && v2::Math::equals(l_cosp, v2::Math::zero_f))
	{
		l_return.Points[0] = 2.0f * atan2f(this->x, this->w);
	}
	else
	{
		l_return.Points[0] = atan2f(l_sinp, l_cosp);
	}

	//yaw
	l_return.Points[1] =
		asinf(v2::Math::clamp(-2.0f * (this->x * this->z - this->w * this->y), -1.0f, 1.0f));

	//roll
	l_return.Points[2] =
		atan2f(2.0f * (this->x * this->y + this->w * this->z), this->w * this->w + this->x * this->x - this->y * this->y - this->z * this->z);


	return l_return;
};

inline quat m33f::to_rotation() const
{
	const v3f& l_right = this->Right;
	const v3f& l_up = this->Up;
	const v3f& l_forward = this->Forward;

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
		return quat{
			l_biggestDiagonalValue,
			(l_right.y + l_up.x) * mult,
			(l_forward.x + l_right.z) * mult,
			(l_up.z - l_forward.y) * mult };
	}
	break;
	case 1:
	{
		return quat{
			(l_right.y + l_up.x) * mult,
			l_biggestDiagonalValue,
			(l_up.z + l_forward.y) * mult,
			(l_forward.x - l_right.z) * mult };
	}
	break;
	case 2:
	{
		return quat{
			(l_forward.x + l_right.z) * mult,
			(l_up.z + l_forward.y) * mult,
			l_biggestDiagonalValue,
			(l_right.y - l_up.x) * mult };
	}
	break;
	case 3:
	{
		return quat{
			(l_up.z - l_forward.y) * mult,
			(l_forward.x - l_right.z) * mult,
			(l_right.y - l_up.x) * mult,
			l_biggestDiagonalValue };
	}
	break;
	}

	return quat{ 0.0f, 0.0f, 0.0f, 1.0f };
}

#define mat_foreach_element_begin(Dimension) \
for (unsigned char p_column_index = 0; p_column_index < Dimension; p_column_index++) \
{\
    for (unsigned  char p_line_index = 0; p_line_index < Dimension; p_line_index++)\
    {

#define mat_foreach_element_end() \
    } \
}

inline char m33f::operator==(const m33f& p_other) const
{
	mat_foreach_element_begin(3)
		if (!v2::Math::equals(this->Points2D[p_column_index].Points[p_line_index], p_other.Points2D[p_column_index].Points[p_line_index]))
		{
			return 0;
		}
	mat_foreach_element_end();
	return 1;
};

inline v3f& m33f::operator[](const unsigned char p_index)
{
	return this->Points2D[p_index];
};


inline float m44f::mul_line_column(const m44f& p_left, const m44f& p_right, const unsigned char p_column_index, const unsigned char p_line_index)
{
	float l_return = 0;
	for (short int i = 0; i < 4; i++)
	{
		l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points2D[p_column_index].Points[i]);
	}
	return l_return;
};

inline float m44f::mul_line_vec(const m44f& p_left, const v4f& p_right, const unsigned char p_line_index)
{
	float l_return = v2::Math::zero_f;
	for (short int i = 0; i < 4; i++)
	{
		l_return += (p_left.Points2D[i].Points[p_line_index] * p_right.Points[i]);
	}
	return l_return;
};

inline m44f m44f::operator*(const m44f& p_other) const
{
	m44f l_return;
	mat_foreach_element_begin(4)
		l_return[p_column_index][p_line_index] = mul_line_column(*this, p_other, p_column_index, p_line_index);
	mat_foreach_element_end()
		return l_return;
};

inline v4f m44f::operator*(const v4f& p_other) const
{
	v4f l_return;
	for (char p_line_index = 0; p_line_index < 4; p_line_index++)
	{
		l_return[p_line_index] = mul_line_vec(*this, p_other, p_line_index);
	}
	return l_return;
};

inline m44f m44f::operator*(const float p_other) const
{
	m44f l_return;
	mat_foreach_element_begin(4)
		l_return[p_column_index][p_line_index] = this->Points2D[p_column_index].Points[p_line_index] * p_other;
	mat_foreach_element_end()
		return l_return;
};

inline m44f m44f::operator+(const float p_other) const
{
	m44f l_return;
	mat_foreach_element_begin(4)
		l_return[p_column_index][p_line_index] = this->Points2D[p_column_index].Points[p_line_index] + p_other;
	mat_foreach_element_end()
		return l_return;
};

inline char m44f::operator==(const m44f& p_other) const
{
	mat_foreach_element_begin(4)
		if (!v2::Math::equals(this->Points2D[p_column_index].Points[p_line_index], p_other.Points2D[p_column_index].Points[p_line_index]))
		{
			return 0;
		};
	mat_foreach_element_end();

	return 1;
};

inline v4f& m44f::operator[](const unsigned char p_index)
{
	return this->Points2D[p_index];
};

inline const v4f& m44f::operator[](const unsigned char p_index) const
{
	return this->Points2D[p_index];
};


inline float m44f::det(const unsigned char p_column_index, const unsigned char p_line_index) const
{
	m33f l_matDet;
	unsigned char l_matDet_column_counter = 0;
	unsigned char l_matDet_line_counter = 0;

	for (unsigned char l_column_index = 0; l_column_index < 4; l_column_index++)
	{
		if (l_column_index != p_column_index)
		{
			l_matDet_line_counter = 0;
			for (unsigned char l_line_index = 0; l_line_index < 4; l_line_index++)
			{
				if (l_line_index != p_line_index)
				{
					l_matDet.Points2D[l_matDet_column_counter].Points[l_matDet_line_counter] = this->Points2D[l_column_index].Points[l_line_index];
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
};

inline m44f m44f::inv() const
{
	m44f l_return;
	float l_det =
		(this->Points2D[0].Points[0] * this->det(0, 0))
		- (this->Points2D[0].Points[1] * this->det(0, 1))
		+ (this->Points2D[0].Points[2] * this->det(0, 2))
		- (this->Points2D[0].Points[3] * this->det(0, 3));

	{
		l_return._00 = this->det(0, 0);
		l_return._01 = -this->det(1, 0);
		l_return._02 = this->det(2, 0);
		l_return._03 = -this->det(3, 0);
		l_return._10 = -this->det(0, 1);
		l_return._11 = this->det(1, 1);
		l_return._12 = -this->det(2, 1);
		l_return._13 = this->det(3, 1);
		l_return._20 = this->det(0, 2);
		l_return._21 = -this->det(1, 2);
		l_return._22 = this->det(2, 2);
		l_return._23 = -this->det(3, 2);
		l_return._30 = -this->det(0, 3);
		l_return._31 = this->det(1, 3);
		l_return._32 = -this->det(2, 3);
		l_return._33 = this->det(3, 3);
	}

	return l_return * (1.0f / l_det);
};

inline const v3f& m44f::get_translation() const
{
	return this->Col3.Vec3;
};

inline m44f m44f::build_translation(const v3f& p_translation)
{
	m44f l_return = m44f_const::IDENTITY;
	l_return.Col3.Vec3 = p_translation;
	return l_return;
};

inline m44f m44f::build_rotation(const m33f& p_axis)
{
	return m44f::build_columns(
		v4f::build_v3f_s(p_axis.Points2D[0], 0.0f),
		v4f::build_v3f_s(p_axis.Points2D[1], 0.0f),
		v4f::build_v3f_s(p_axis.Points2D[2], 0.0f),
		v4f{0.0f, 0.0f, 0.0f, 1.0f}
	);
};

inline m44f m44f::build_rotation(const v3f& p_right, const v3f& p_up, const v3f& p_forward)
{
	return m44f::build_columns(
		v4f::build_v3f_s(p_right, 0.0f),
		v4f::build_v3f_s(p_up, 0.0f),
		v4f::build_v3f_s(p_forward, 0.0f),
		v4f{ 0.0f, 0.0f, 0.0f, 1.0f }
	);
};

inline m44f m44f::build_scale(const v3f& p_scale)
{
	return m44f::build_columns(
		v4f{ p_scale.x, 0.0f, 0.0f, 0.0f },
		v4f{ 0.0f, p_scale.y, 0.0f, 0.0f },
		v4f{ 0.0f, 0.0f, p_scale.z, 0.0f },
		v4f{ 0.0f, 0.0f, 0.0f, 1.0f }
	);
};


inline m44f m44f::trs(const m44f& p_translation, const m44f& p_rotation, const m44f& p_scale)
{
	return (p_translation * p_rotation)* p_scale;
};

inline m44f m44f::trs(const v3f& p_translation, const m33f& p_axis, const v3f& p_scale)
{
	return m44f::trs(m44f::build_translation(p_translation), m44f::build_rotation(p_axis), m44f::build_scale(p_scale));
};

inline m44f m44f::lookat_rotation(const v3f& p_origin, const v3f& p_target, const v3f& p_up)
{
	m44f l_return = m44f_const::IDENTITY;

	l_return.Forward.Vec3 = (p_target - p_origin).normalize();
	l_return.Right.Vec3 = l_return.Forward.Vec3.cross(p_up).normalize();
	l_return.Up.Vec3 = l_return.Right.Vec3.cross(l_return.Forward.Vec3).normalize();

	return l_return;
};

inline m44f m44f::lookat_rotation_inverted(const v3f& p_origin, const v3f& p_target, const v3f& p_up)
{
	m44f l_return = m44f_const::IDENTITY;

	l_return.Forward.Vec3 = (p_target - p_origin).normalize();
	l_return.Right.Vec3 = l_return.Forward.Vec3.cross(p_up).normalize() * -1.0f;
	l_return.Up.Vec3 = l_return.Right.Vec3.cross(l_return.Forward.Vec3).normalize() * -1.0f;

	return l_return;
};

inline m44f m44f::view(const v3f& p_world_position, const v3f& p_forward, const v3f& p_up)
{
	return m44f::view_normalized(p_world_position, p_forward, p_up.normalize());
};


inline m44f m44f::view_normalized(const v3f& p_world_position, const v3f& p_forward, const v3f& p_up_normalized)
{
#ifdef MATH_NORMALIZATION_TEST
	v3f_assert_is_normalized(p_up_normalized);
#endif
	v3f l_target = p_forward;
	l_target = p_world_position + l_target;
	v3f l_up = p_up_normalized * -1.0f;
	return m44f::trs(m44f::build_translation(p_world_position), m44f::lookat_rotation_inverted(p_world_position, l_target, l_up), m44f::build_scale(v3f_const::ONE))
					.inv();
};

inline m44f m44f::perspective(const float p_fov, const float p_aspect, const float p_near, const float p_far)
{
	m44f l_return;
	float l_halfTan = tanf(p_fov / 2.0f);

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
};


inline char transform::operator==(const transform& p_other)
{
	return (this->position == p_other.position)
		& (this->rotation == p_other.rotation)
		& (this->scale == p_other.scale);
};