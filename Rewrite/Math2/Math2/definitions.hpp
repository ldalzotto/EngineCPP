#pragma once

namespace v2
{
	struct Math
	{
		constexpr static const float tol_f = 0.000001f;
		constexpr static const float zero_f = 0.0f;
		constexpr static const float one_f = 1.0f;
		constexpr static const float M_PI = 3.14159265358979323846f;
		constexpr static const float DEG_TO_RAD = (M_PI / 180.0f);
		constexpr static const float RAD_TO_DEG = (180.0f / M_PI);

		inline static char equals(const float p_left, const float p_right);
		inline static char lower_eq(const float p_left, const float p_right);
		inline static char lower(const float p_left, const float p_right);
		inline static char greater_eq(const float p_left, const float p_right);
		inline static char greater(const float p_left, const float p_right);
		inline static short int sign(const float p_value);
		inline static float clamp(const float p_value, const float p_left, const float p_right);
	};
}

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

	v2f operator*(const v2f& p_other);
	char operator==(const v2f& p_other);
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

	v3f operator+(const v3f& p_other) const;
	v3f operator*(const float p_other) const;
	v3f operator*(const v3f& p_other) const;
	v3f operator-(const v3f& p_other) const;
	char operator==(const v3f& p_other) const;
	float& operator[](const unsigned char p_index);
	float dot(const v3f& p_other) const;
	v3f cross(const v3f& p_other) const;
	float length() const;
	v3f normalize() const;
	v3f project(const v3f& p_projected_on) const;
	v3f project_normalized(const v3f& p_projected_on) const;
	float distance(const v3f& p_end) const;
	float angle_unsigned(const v3f& p_end) const;
	float angle_unsigned_normalized(const v3f& p_end_normalized) const;
	char anglesign(const v3f& p_end, const v3f& p_ref_axis) const;
	v3f rotate(const quat& p_rotation) const;
	quat euler_to_quat() const;
	quat from_to(const v3f& p_to) const;
	quat from_to_normalized(const v3f& p_to) const;
};

struct v3f_const
{
	static constexpr const v3f ZERO = { 0.0f, 0.0f, 0.0f };
	static constexpr const v3f ONE = { 1.0f, 1.0f, 1.0f };
	static constexpr const v3f RIGHT = { 1.0f, 0.0f, 0.0f };
	static constexpr const v3f UP = { 0.0f, 1.0f, 0.0f };
	static constexpr const v3f FORWARD = { 0.0f, 0.0f, 1.0f };
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

	inline static v4f build_v3f_s(const v3f& p_v3f, const float p_s)
	{
		return v4f{p_v3f.x,p_v3f.y ,p_v3f.z , p_s};
	};

	char operator==(const v4f& p_other) const;
	v4f operator*(const float p_other) const;
	v4f operator*(const v4f& p_other) const;
	float& operator[](const unsigned char p_index);
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

	inline static quat build_v3f_f(const v3f& p_vec, const float p_scal)
	{
		return quat{
			p_vec.x, p_vec.y, p_vec.z, p_scal };
	};

	inline static quat build_v4f(const v4f& p_quat)
	{
		return quat{ p_quat.x, p_quat.y, p_quat.z, p_quat.w };
	};

	static quat rotate_around(const v3f& p_axis, const float p_angle);

	char operator==(const quat& p_other) const;
	quat operator*(const quat& p_other) const;
	quat normalize() const;
	quat inv() const;
	quat cross(const quat& p_other) const;
	m33f to_axis() const; // Converts a quat rotation to 3D axis
	v3f euler() const;
};

struct quat_const
{
	static constexpr const quat IDENTITY = quat{ 0.0f, 0.0f, 0.0f, 1.0f };
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

	inline static m33f build_columns(const v3f& p_right, const v3f& p_up, const v3f& p_forward)
	{
		return m33f{
			p_right.x, p_right.y, p_right.z,
			p_up.x, p_up.y, p_up.z,
			p_forward.x, p_forward.y, p_forward.z };
	};

	char operator==(const m33f& p_other) const;
	v3f& operator[](const unsigned char p_index);
	quat to_rotation() const; // Converts a 3D axis matrix to rotation as quaternion
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

	inline static m44f build_columns(const v4f& p_col0, const v4f& p_col1, const v4f& p_col2, const v4f& p_col3)
	{
		return m44f
		{
			p_col0.x, p_col0.y,p_col0.z,p_col0.w,
			p_col1.x, p_col1.y,p_col1.z,p_col1.w,
			p_col2.x, p_col2.y,p_col2.z,p_col2.w,
			p_col3.x, p_col3.y,p_col3.z,p_col3.w
		};
	};

	static float mul_line_column(const m44f& p_left, const m44f& p_right, const unsigned char p_column_index, const unsigned char p_line_index);
	static float mul_line_vec(const m44f& p_left, const v4f& p_right, const unsigned char p_line_index);

	m44f operator*(const m44f& p_other) const;
	v4f operator*(const v4f& p_other) const;
	m44f operator*(const float p_other) const;
	m44f operator+(const float p_other) const;
	char operator==(const m44f& p_other) const;
	v4f& operator[](const unsigned char p_index);
	const v4f& operator[](const unsigned char p_index) const;

	float det(const unsigned char p_column_index, const unsigned char p_line_index) const;
	m44f inv() const;

	static m44f build_translation(const v3f& p_translation);
	static m44f build_rotation(const m33f& p_axis);
	static m44f build_rotation(const v3f& p_right, const v3f& p_up, const v3f& p_forward);
	static m44f build_scale(const v3f& p_scale);

	static m44f trs(const m44f& p_translation, const m44f& p_rotation, const m44f& p_scale);
	static m44f trs(const v3f& p_translation, const m33f& p_axis, const v3f& p_scale);

	static m44f lookat_rotation(const v3f& p_origin, const v3f& p_target, const v3f& p_up);

	//If p_up is already normalized, we can use the cheaper version : lookat_rotation_inverted_normalized
	static m44f lookat_rotation_inverted(const v3f& p_origin, const v3f& p_target, const v3f& p_up);

	//If p_up is already normalized, we can use the cheaper version : view_normalized
	static m44f view(const v3f& p_world_position, const v3f& p_forward, const v3f& p_up);
	static m44f view_normalized(const v3f& p_world_position, const v3f& p_forward, const v3f& p_up_normalized);

	static m44f perspective(const float p_fov, const float p_aspect, const float p_near, const float p_far);
};

struct m44f_const
{
	inline static const m44f IDENTITY = m44f::build_columns(
		v4f{ 1.0f,0.0f,0.0f,0.0f },
		v4f{ 0.0f,1.0f,0.0f,0.0f },
		v4f{ 0.0f,0.0f,1.0f,0.0f },
		v4f{ 0.0f,0.0f,0.0f,1.0f }
	);
};

struct transform
{
	v3f position;
	quat rotation;
	v3f scale;
};

struct transform_const
{
	inline static const transform ORIGIN = transform{
		v3f_const::ZERO,
		quat_const::IDENTITY,
		v3f_const::ONE
	};
};

struct transform_pa
{
	v3f position;
	m33f axis;
};
