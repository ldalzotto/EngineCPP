#pragma once

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
#define math_square_op(Left) Left *Left

inline char v2f::operator==(const v2f &p_other)
{
    return Math::equals(this->Points[0], p_other.Points[0]) &&
           Math::equals(this->Points[1], p_other.Points[1]);
};

inline v2f v2f::operator*(const v2f &p_other)
{
    return math_v2f_foreach_2(this, &p_other, math_mul_op);
};

inline v3f v3f::operator+(const v3f &p_other) const
{
    return math_v3f_foreach_2(this, &p_other, math_add_op);
};

inline v3f v3f::operator*(const float p_other) const
{
    return v3f{this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other};
};

inline v3f v3f::operator*(const v3f &p_other) const
{
    return math_v3f_foreach_2(this, &p_other, math_mul_op);
};

inline v3f v3f::operator-(const v3f &p_other) const
{
    return math_v3f_foreach_2(this, &p_other, math_min_op);
};

inline char v3f::operator==(const v3f &p_other) const
{
    return Math::equals(this->Points[0], p_other.Points[0]) &&
           Math::equals(this->Points[1], p_other.Points[1]) &&
           Math::equals(this->Points[2], p_other.Points[2]);
};

inline float v3f::dot(const v3f &p_other) const
{
    v3f l_mul = this->operator*(p_other);
    return math_v3f_reduce(&l_mul, math_add_op);
};

inline v3f v3f::cross(const v3f &p_other) const
{
    return v3f{
        (this->Points[1] * p_other.Points[2]) - (this->Points[2] * p_other.Points[1]),
        (this->Points[2] * p_other.Points[0]) - (this->Points[0] * p_other.Points[2]),
        (this->Points[0] * p_other.Points[1]) - (this->Points[1] * p_other.Points[0])};
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

inline v3f v3f::project(const v3f &p_projected_on) const
{
    return p_projected_on * (this->dot(p_projected_on) / p_projected_on.length());
};

inline float v3f::distance(const v3f &p_end) const
{
    return this->operator-(p_end).length();
};

inline float v3f::angle(const v3f &p_end) const
{
    return acosf(this->dot(p_end) / (this->length() * p_end.length()));
};

inline float v3f::angle_normalized(const v3f &p_end_normalized) const
{
#ifdef MATH_BOUND_TEST
    //Inputs are not normalized
    if (this->length() < Math::tol_f || p_end_normalized.length() < Math::tol_f)
    {
        abort();
    }
#endif
    return acosf(this->dot(p_end_normalized));
};

inline char v3f::anglesign(const v3f &p_end, const v3f &p_ref_axis) const
{
    float l_dot = this->cross(p_end).dot(p_ref_axis);
    return l_dot >= Math::tol_f ? 1 : -1;
};

inline v3f v3f::rotate(const quat &p_rotation) const
{
    return (p_rotation * quat::build_v3f_f(*this, 0.0f) * p_rotation.inv()).Vec3s.Vec * this->length();
};

inline char v4f::operator==(const v4f &p_other) const
{
    return Math::equals(this->Points[0], p_other.Points[0]) &&
           Math::equals(this->Points[1], p_other.Points[1]) &&
           Math::equals(this->Points[2], p_other.Points[2]) &&
           Math::equals(this->Points[3], p_other.Points[3]);
};

inline v4f v4f::operator*(const float p_other) const
{
    return v4f{this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other, this->Points[3] * p_other};
};

inline v4f v4f::operator*(const v4f &p_other) const
{
    return math_v4f_foreach_2(this, &p_other, math_mul_op);
};

inline float v4f::length() const
{
    v4f l_squared = math_v4f_foreach(this, math_square_op);
    return sqrtf(math_v4f_reduce(&l_squared, math_add_op));
};

inline quat quat::rotate_around(const v3f &p_axis, const float p_angle)
{
    return quat::build_v3f_f(p_axis * sinf(p_angle * 0.5f), cosf(p_angle * 0.5f));
};

inline char quat::operator==(const quat &p_other) const
{
    return this->Points == p_other.Points;
};

inline quat quat::operator*(const quat &p_other) const
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

inline quat quat::cross(const quat &p_other) const
{
    v3f l_rotated_left = v3f_const::FORWARD.rotate(*this).normalize();
    v3f l_rotated_right = v3f_const::FORWARD.rotate(p_other).normalize();
    return quat::rotate_around(l_rotated_left.cross(l_rotated_right), 0.0f);
};

inline m33f quat::rotation_to_axis() const
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
        (2 * l_qxz) - (2 * l_qyw)};

    //UP
    l_return.Col1 = v3f{
        (2 * l_qxy) - (2 * l_qzw),
        1 - (2 * l_qxx) - (2 * l_qzz),
        (2 * l_qyz) + (2 * l_qxw)};

    //Forward
    l_return.Col2 = v3f{
        (2 * l_qxz) + (2 * l_qyw),
        (2 * l_qyz) - (2 * l_qxw),
        1 - (2 * l_qxx) - (2 * l_qyy)};

    l_return.Col0 = l_return.Col0.normalize();
    l_return.Col1 = l_return.Col1.normalize();
    l_return.Col2 = l_return.Col2.normalize();

    return l_return;
};

inline quat m33f::axis_to_rotation() const
{
    const v3f &l_right = this->Right;
    const v3f &l_up = this->Up;
    const v3f &l_forward = this->Forward;

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
            (l_up.z - l_forward.y) * mult};
    }
    break;
    case 1:
    {
        return quat{
            (l_right.y + l_up.x) * mult,
            l_biggestDiagonalValue,
            (l_up.z + l_forward.y) * mult,
            (l_forward.x - l_right.z) * mult};
    }
    break;
    case 2:
    {
        return quat{
            (l_forward.x + l_right.z) * mult,
            (l_up.z + l_forward.y) * mult,
            l_biggestDiagonalValue,
            (l_right.y - l_up.x) * mult};
    }
    break;
    case 3:
    {
        return quat{
            (l_up.z - l_forward.y) * mult,
            (l_forward.x - l_right.z) * mult,
            (l_right.y - l_up.x) * mult,
            l_biggestDiagonalValue};
    }
    break;
    }

    return quat{0.0f, 0.0f, 0.0f, 1.0f};
}

inline char m33f::operator==(const m33f &p_other)
{
    for (char i = 0; i < 3; i++)
    {
        for (char j = 0; j < 3; j++)
        {
            if(!Math::equals(this->Points2D[i].Points[j], p_other.Points2D[i].Points[j]))
            {
                return 0;
            }
        }
    }
    return 1;
};
