#pragma once

template <class TYPE>
struct Tolerance{};

template <>
struct Tolerance<float>
{
	static constexpr const float tol = 0.000001f;
};


template <class TYPE>
struct Zero {};

template <>
struct Zero<float>
{
	static constexpr const float zer = 0.0f;
};

template <class TYPE>
struct One {};

template <>
struct One<float>
{
	static constexpr const float one = 1.0f;
};

constexpr const float M_PI = 3.14159265358979323846f;
constexpr const float DEG_TO_RAD = (M_PI / 180.0f);
constexpr const float RAD_TO_DEG = (180.0f / M_PI);