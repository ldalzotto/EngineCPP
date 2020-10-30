#pragma once

namespace Math
{

	template <unsigned N, class TYPE>
	struct Vector
	{
		
	};

	template <class TYPE>
	struct alignas(sizeof(TYPE)) Vector<3, TYPE>
	{
		union
		{
			TYPE Points[3];
			struct { TYPE x, y, z; };
		};

		Vector() = default;
		
		inline Vector(TYPE p_x, TYPE p_y, TYPE p_z) : x{ p_x }, y{ p_y }, z{ p_z }
		{
		}

		inline Vector<3, TYPE> operator*(const Vector<3, TYPE>& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] * p_other.Points[0], this->Points[1] * p_other.Points[1], this->Points[2] * p_other.Points[2]);
		}

		inline Vector<3, TYPE> operator*(const TYPE& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other);
		}

		inline Vector<3, TYPE> operator/(const Vector<3, TYPE>& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] / p_other.Points[0], this->Points[1] / p_other.Points[1], this->Points[2] / p_other.Points[2]);
		}

		inline Vector<3, TYPE> operator/(const TYPE& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] / p_other, this->Points[1] / p_other, this->Points[2] / p_other);
		}

		inline Vector<3, TYPE> operator+(const Vector<3, TYPE>& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] + p_other.Points[0], this->Points[1] + p_other.Points[1], this->Points[2] + p_other.Points[2]);
		}

		inline Vector<3, TYPE> operator+(const TYPE& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] + p_other, this->Points[1] + p_other, this->Points[2] + p_other);
		}

		inline Vector<3, TYPE> operator-(const Vector<3, TYPE>& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] - p_other.Points[0], this->Points[1] - p_other.Points[1], this->Points[2] - p_other.Points[2]);
		}

		inline Vector<3, TYPE> operator-(const TYPE& p_other)
		{
			return Vector<3, TYPE>(this->Points[0] - p_other, this->Points[1] - p_other, this->Points[2] - p_other);
		}

		inline TYPE& operator[](int p_index)
		{
			return this->Points[p_index];
		}
	};

	template <class TYPE>
	struct alignas(sizeof(TYPE)) Vector<4, TYPE>
	{

		union
		{
			TYPE Points[4];
			struct { TYPE x; TYPE y; TYPE z; TYPE w; };
			struct { Vector<3, TYPE> Vec3; TYPE Vec3_w; };
		};

		Vector() = default;
		inline Vector(TYPE p_x, TYPE p_y, TYPE p_z, TYPE p_w) : x{p_x}, y{p_y}, z{p_z}, w{p_w}{}
		inline Vector(const Vector<3, TYPE>& p_vec3, const TYPE& p_w) : Vec3{ p_vec3 }, Vec3_w{ p_w }{};

		inline Vector<4, TYPE> operator*(const Vector<4, TYPE>& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] * p_other.Points[0], this->Points[1] * p_other.Points[1], this->Points[2] * p_other.Points[2], this->Points[3] * p_other.Points[3]);
		}

		inline Vector<4, TYPE> operator*(const TYPE& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other, this->Points[3] * p_other);
		}

		inline Vector<4, TYPE> operator/(const Vector<4, TYPE>& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] / p_other.Points[0], this->Points[1] / p_other.Points[1], this->Points[2] / p_other.Points[2], this->Points[3] / p_other.Points[3]);
		}

		inline Vector<4, TYPE> operator/(const TYPE& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] / p_other, this->Points[1] / p_other, this->Points[2] / p_other, this->Points[3] / p_other);
		}

		inline Vector<4, TYPE> operator+(const Vector<4, TYPE>& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] + p_other.Points[0], this->Points[1] + p_other.Points[1], this->Points[2] + p_other.Points[2], this->Points[3] + p_other.Points[3]);
		}

		inline Vector<4, TYPE> operator+(const TYPE& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] + p_other, this->Points[1] + p_other, this->Points[2] + p_other, this->Points[3] + p_other);
		}

		inline Vector<4, TYPE> operator-(const Vector<4, TYPE>& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] - p_other.Points[0], this->Points[1] - p_other.Points[1], this->Points[2] - p_other.Points[2], this->Points[3] - p_other.Points[3]);
		}

		inline Vector<4, TYPE> operator-(const TYPE& p_other)
		{
			return Vector<4, TYPE>(this->Points[0] - p_other, this->Points[1] - p_other, this->Points[2] - p_other, this->Points[3] - p_other);
		}

		inline TYPE& operator[](int p_index)
		{
			return this->Points[p_index];
		}
	};

	using vec3f = Vector<3, float>;
	using vec4f = Vector<4, float>;

	const vec3f vec3f_ZERO = { 0.0f, 0.0f, 0.0f };
	const vec3f vec3f_RIGHT = { 1.0f, 0.0f, 0.0f };
	const vec3f vec3f_UP = { 0.0f, 1.0f, 0.0f };
	const vec3f vec3f_FORWARD = { 0.0f, 0.0f, 1.0f };
}