#pragma once

struct Math
{
    constexpr static const float tol_f = 0.000001f;
    constexpr static const float zero_f = 0.0f;
    constexpr static const float one_f = 1.0f;
    constexpr static const float M_PI = 3.14159265358979323846f;
    constexpr static const float DEG_TO_RAD = (M_PI / 180.0f);
    constexpr static const float RAD_TO_DEG = (180.0f / M_PI);

    inline static char equals(const float p_left, const float p_right)
    {
        return fabsf(p_left - p_right) <= tol_f;
    };

    inline short int sign(const float p_value)
    {
        if (p_value <= -tol_f)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    };

    inline float clamp(const float p_value, const float p_left, const float p_right)
    {
        if (p_value >= (p_right + tol_f))
        {
            return p_right;
        }
        else if (p_value <= (p_left + tol_f))
        {
            return p_left;
        }
        return p_value;
    };
};

struct v2f;
struct v3f;
struct v4f;
struct quat;
struct m44f;
struct m33f;

struct alignas(sizeof(float)) v2f
{
    union
    {
        float Points[2];
        struct
        {
            float x, y;
        };
    };

    v2f operator*(const v2f &p_other);
    char operator==(const v2f &p_other);
};

struct alignas(sizeof(float)) v3f
{
    union
    {
        float Points[3];
        struct
        {
            float x, y, z;
        };
    };

    v3f operator+(const v3f &p_other) const;
    v3f operator*(const float p_other) const;
    v3f operator*(const v3f &p_other) const;
    v3f operator-(const v3f &p_other) const;
    char operator==(const v3f &p_other) const;
    float dot(const v3f &p_other) const;
    v3f cross(const v3f &p_other) const;
    float length() const;
    v3f normalize() const;
    v3f project(const v3f &p_projected_on) const;
    float distance(const v3f &p_end) const;
    float angle(const v3f &p_end) const;
    float angle_normalized(const v3f &p_end_normalized) const;
    char anglesign(const v3f &p_end, const v3f &p_ref_axis) const;
    v3f rotate(const quat &p_rotation) const;
};

struct v3f_const
{
    static constexpr const v3f ZERO = {0.0f, 0.0f, 0.0f};
    static constexpr const v3f ONE = {1.0f, 1.0f, 1.0f};
    static constexpr const v3f RIGHT = {1.0f, 0.0f, 0.0f};
    static constexpr const v3f UP = {0.0f, 1.0f, 0.0f};
    static constexpr const v3f FORWARD = {0.0f, 0.0f, 1.0f};
};

struct alignas(sizeof(float)) v4f
{
    union
    {
        float Points[4];
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            v3f Vec3;
            float Vec3_w;
        };
    };

    char operator==(const v4f &p_other) const;
    v4f operator*(const float p_other) const;
    v4f operator*(const v4f &p_other) const;
    float length() const;
};

struct quat
{
    union
    {
        v4f Points;
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            v3f Vec;
            float Scal;
        } Vec3s;
    };

    inline static quat build_v3f_f(const v3f &p_vec, const float p_scal)
    {
        return quat{
            p_vec.x, p_vec.y, p_vec.z, p_scal};
    };

    inline static quat build_v4f(const v4f &p_quat)
    {
        return quat{p_quat.x, p_quat.y, p_quat.z, p_quat.w};
    };

    static quat rotate_around(const v3f &p_axis, const float p_angle);

    char operator==(const quat &p_other) const;
    quat operator*(const quat &p_other) const;
    quat normalize() const;
    quat inv() const;
    quat cross(const quat &p_other) const;
    m33f rotation_to_axis() const;
};

struct alignas(sizeof(float)) m33f
{
    union
    {
        float Points[9];
        v3f Points2D[3];
        struct
        {
            float _00, _01, _02, _10, _11, _12, _20, _21, _22;
        };
        struct
        {
            v3f Col0, Col1, Col2;
        };
        struct
        {
            v3f Right, Up, Forward;
        };
    };

    inline static m33f build_columns(const v3f &p_right, const v3f &p_up, const v3f &p_forward)
    {
        return m33f{
            p_right.x, p_right.y, p_right.z,
            p_up.x, p_up.y, p_up.z,
            p_forward.x, p_forward.y, p_forward.z};
    };

    char operator==(const m33f &p_other);
    quat axis_to_rotation() const;
};

struct alignas(sizeof(float)) m44f
{
    union
    {
        float Points[16];
        v4f Points2D[4];
        struct
        {
            float _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33;
        };
        struct
        {
            v4f Col0, Col1, Col2, Col3;
        };
        struct
        {
            v4f Right, Up, Forward, Col3_Direction;
        };
    };
};