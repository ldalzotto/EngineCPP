#pragma once

namespace v2
{
	struct Math
	{
		constexpr static const float32 tol_f = 0.000001f;
		constexpr static const float32 zero_f = 0.0f;
		constexpr static const float32 one_f = 1.0f;
		constexpr static const float32 M_PI = 3.14159265358979323846f;
		constexpr static const float32 DEG_TO_RAD = (M_PI / 180.0f);
		constexpr static const float32 RAD_TO_DEG = (180.0f / M_PI);

		inline static int8 equals(const float32 p_left, const float32 p_right);
		inline static int8 nequals(const float32 p_left, const float32 p_right);
		inline static int8 lower_eq(const float32 p_left, const float32 p_right);
		inline static int8 lower(const float32 p_left, const float32 p_right);
		inline static int8 greater_eq(const float32 p_left, const float32 p_right);
		inline static int8 greater(const float32 p_left, const float32 p_right);
		inline static int16 sign(const float32 p_value);
		inline static float32 clamp(const float32 p_value, const float32 p_left, const float32 p_right);
	};
}

struct v2f;
struct v3f;
struct v4f;
struct quat;
struct m44f;
struct m33f;

struct alignas(sizeof(float32)) v2f
{
	union
	{
		float32 Points[2];
		struct
		{
			float32 x, y;
		};
	};

	v2f operator*(const v2f& p_other);
	int8 operator==(const v2f& p_other);
};

struct alignas(sizeof(float32)) v3f
{
	union
	{
		float32 Points[3];
		struct
		{
			float32 x, y, z;
		};
	};

	v3f operator+(const v3f& p_other) const;
	v3f operator*(const float32 p_other) const;
	v3f operator*(const v3f& p_other) const;
	v3f operator-(const v3f& p_other) const;
	int8 operator==(const v3f& p_other) const;
	int8 operator!=(const v3f& p_other) const;
	float32& operator[](const uint8 p_index);
	float32 dot(const v3f& p_other) const;
	v3f cross(const v3f& p_other) const;
	float32 length() const;
	v3f normalize() const;
	v3f inv() const;
	v3f project(const v3f& p_projected_on) const;
	v3f project_normalized(const v3f& p_projected_on) const;
	float32 distance(const v3f& p_end) const;
	float32 angle_unsigned(const v3f& p_end) const;
	float32 angle_unsigned_normalized(const v3f& p_end_normalized) const;
	int8 anglesign(const v3f& p_end, const v3f& p_ref_axis) const;
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

struct alignas(sizeof(float32)) v4f
{
	union
	{
		float32 Points[4];
		struct
		{
			float32 x, y, z, w;
		};
		struct
		{
			v3f Vec3;
			float32 Vec3_w;
		};
	};

	inline static v4f build_v3f_s(const v3f& p_v3f, const float32 p_s)
	{
		return v4f{ p_v3f.x,p_v3f.y ,p_v3f.z , p_s };
	};

	int8 operator==(const v4f& p_other) const;
	int8 operator!=(const v4f& p_other) const;
	v4f operator*(const float32 p_other) const;
	v4f operator*(const v4f& p_other) const;
	float32& operator[](const uint8 p_index);
	float32 length() const;
};

struct quat
{
	union
	{
		v4f Points;
		struct
		{
			float32 x, y, z, w;
		};
		struct
		{
			v3f Vec;
			float32 Scal;
		} Vec3s;
	};

	inline static quat build_v3f_f(const v3f& p_vec, const float32 p_scal)
	{
		return quat{
			p_vec.x, p_vec.y, p_vec.z, p_scal };
	};

	inline static quat build_v4f(const v4f& p_quat)
	{
		return quat{ p_quat.x, p_quat.y, p_quat.z, p_quat.w };
	};

	static quat rotate_around(const v3f& p_axis, const float32 p_angle);

	int8 operator==(const quat& p_other) const;
	int8 operator!=(const quat& p_other) const;
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

struct alignas(sizeof(float32)) m33f
{
	union
	{
		float32 Points[9];
		v3f Points2D[3];
		struct
		{
			float32 _00, _01, _02, _10, _11, _12, _20, _21, _22;
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

	int8 operator==(const m33f& p_other) const;
	v3f& operator[](const uint8 p_index);
	quat to_rotation() const; // Converts a 3D axis matrix to rotation as quaternion
};

struct alignas(sizeof(float32)) m44f
{
	union
	{
		float32 Points[16];
		v4f Points2D[4];
		struct
		{
			float32 _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33;
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

	static float32 mul_line_column(const m44f& p_left, const m44f& p_right, const uint8 p_column_index, const uint8 p_line_index);
	static float32 mul_line_vec(const m44f& p_left, const v4f& p_right, const uint8 p_line_index);

	m44f operator*(const m44f& p_other) const;
	v4f operator*(const v4f& p_other) const;
	m44f operator*(const float32 p_other) const;
	m44f operator+(const float32 p_other) const;
	int8 operator==(const m44f& p_other) const;
	v4f& operator[](const uint8 p_index);
	const v4f& operator[](const uint8 p_index) const;

	float32 det(const uint8 p_column_index, const uint8 p_line_index) const;
	m44f inv() const;

	const v3f& get_translation() const;

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

	static m44f perspective(const float32 p_fov, const float32 p_aspect, const float32 p_near, const float32 p_far);
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

	int8 operator==(const transform& p_other);
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
