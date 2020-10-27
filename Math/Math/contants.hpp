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
