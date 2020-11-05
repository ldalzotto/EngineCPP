#pragma once

template <class TYPE>
struct Tolerance{};

template <>
struct Tolerance<float>
{
	static const float tol;
};


template <class TYPE>
struct Zero {};

template <>
struct Zero<float>
{
	static const float zer;
};

template <class TYPE>
struct One {};

template <>
struct One<float>
{
	static const float one;
};

extern const float M_PI;
extern const float RAD_TO_DEG;
extern const float DEG_TO_RAD;