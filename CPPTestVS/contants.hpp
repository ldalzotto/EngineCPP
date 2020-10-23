#pragma once

template <class TYPE>
struct Tolerance{};

template <>
struct Tolerance<float>
{
	static const float tol;
};

const float Tolerance<float>::tol = 0.000001f;


template <class TYPE>
struct Zero {};

template <>
struct Zero<float>
{
	static const float zer;
};

const float Zero<float>::zer = 0.0f;
