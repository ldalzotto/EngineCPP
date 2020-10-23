#pragma once

namespace Math
{

	template <unsigned N, class TYPE>
	struct Vector
	{
		TYPE Points[N];
	};

	template <class TYPE>
	struct Vector3 : public Vector<3, TYPE>
	{
		inline Vector3(TYPE p_x, TYPE p_y, TYPE p_z)
		{
			this->Points[0] = p_x;
			this->Points[1] = p_y;
			this->Points[2] = p_z;
		}

		inline Vector3<TYPE> operator*(const Vector3<TYPE>& p_other)
		{
			return Vector3<TYPE>(this->Points[0] * p_other.Points[0], this->Points[1] * p_other.Points[1], this->Points[2] * p_other.Points[2]);
		}

		inline Vector3<TYPE> operator*(const TYPE& p_other)
		{
			return Vector3<TYPE>(this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other);
		}

		inline Vector3<TYPE> operator/(const Vector3<TYPE>& p_other)
		{
			return Vector3<TYPE>(this->Points[0] / p_other.Points[0], this->Points[1] / p_other.Points[1], this->Points[2] / p_other.Points[2]);
		}

		inline Vector3<TYPE> operator/(const TYPE& p_other)
		{
			return Vector3<TYPE>(this->Points[0] / p_other, this->Points[1] / p_other, this->Points[2] / p_other);
		}

		inline Vector3<TYPE> operator+(const Vector3<TYPE>& p_other)
		{
			return Vector3<TYPE>(this->Points[0] + p_other.Points[0], this->Points[1] + p_other.Points[1], this->Points[2] + p_other.Points[2]);
		}

		inline Vector3<TYPE> operator+(const TYPE& p_other)
		{
			return Vector3<TYPE>(this->Points[0] + p_other, this->Points[1] + p_other, this->Points[2] + p_other);
		}

		inline Vector3<TYPE> operator-(const Vector3<TYPE>& p_other)
		{
			return Vector3<TYPE>(this->Points[0] - p_other.Points[0], this->Points[1] - p_other.Points[1], this->Points[2] - p_other.Points[2]);
		}

		inline Vector3<TYPE> operator-(const TYPE& p_other)
		{
			return Vector3<TYPE>(this->Points[0] - p_other, this->Points[1] - p_other, this->Points[2] - p_other);
		}
	};

	using vec3f = Vector3<float>;


	template <class TYPE>
	struct Vector4 : public Vector<4, TYPE>
	{
		inline Vector4(TYPE p_x, TYPE p_y, TYPE p_z, TYPE p_w)
		{
			this->Points[0] = p_x;
			this->Points[1] = p_y;
			this->Points[2] = p_z;
			this->Points[3] = p_w;
		}

		inline Vector4<TYPE> operator*(const Vector4<TYPE>& p_other)
		{
			return Vector4<TYPE>(this->Points[0] * p_other.Points[0], this->Points[1] * p_other.Points[1], this->Points[2] * p_other.Points[2], this->Points[3] * p_other.Points[3]);
		}

		inline Vector4<TYPE> operator*(const TYPE& p_other)
		{
			return Vector4<TYPE>(this->Points[0] * p_other, this->Points[1] * p_other, this->Points[2] * p_other, this->Points[3] * p_other);
		}

		inline Vector4<TYPE> operator/(const Vector4<TYPE>& p_other)
		{
			return Vector4<TYPE>(this->Points[0] / p_other.Points[0], this->Points[1] / p_other.Points[1], this->Points[2] / p_other.Points[2], this->Points[3] / p_other.Points[3]);
		}

		inline Vector4<TYPE> operator/(const TYPE& p_other)
		{
			return Vector4<TYPE>(this->Points[0] / p_other, this->Points[1] / p_other, this->Points[2] / p_other, this->Points[3] / p_other);
		}

		inline Vector4<TYPE> operator+(const Vector4<TYPE>& p_other)
		{
			return Vector4<TYPE>(this->Points[0] + p_other.Points[0], this->Points[1] + p_other.Points[1], this->Points[2] + p_other.Points[2], this->Points[3] + p_other.Points[3]);
		}

		inline Vector4<TYPE> operator+(const TYPE& p_other)
		{
			return Vector4<TYPE>(this->Points[0] + p_other, this->Points[1] + p_other, this->Points[2] + p_other, this->Points[3] + p_other);
		}

		inline Vector4<TYPE> operator-(const Vector4<TYPE>& p_other)
		{
			return Vector4<TYPE>(this->Points[0] - p_other.Points[0], this->Points[1] - p_other.Points[1], this->Points[2] - p_other.Points[2], this->Points[3] - p_other.Points[3]);
		}

		inline Vector4<TYPE> operator-(const TYPE& p_other)
		{
			return Vector4<TYPE>(this->Points[0] - p_other, this->Points[1] - p_other, this->Points[2] - p_other, this->Points[3] - p_other);
		}
	};

	using vec4f = Vector4<float>;

	const Vector<3, float> vec3f_ZERO = { 0.0f, 0.0f, 0.0f };
	const Vector<3, float> vec3f_RIGHT = { 1.0f, 0.0f, 0.0f };
	const Vector<3, float> vec3f_UP = { 0.0f, 1.0f, 0.0f };
	const Vector<3, float> vec3f_FORWARD = { 0.0f, 0.0f, 1.0f };
}