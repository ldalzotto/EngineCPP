#pragma once

template <class TYPE>
struct Array
{
	TYPE* Memory;
	size_t Capacity;

	inline Array() { this->Memory = nullptr; this->Capacity = 0; }
	inline Array(TYPE* p_memory, const size_t p_capacity) : Memory{p_memory}, Capacity{p_capacity} { }

	inline TYPE& operator[](size_t p_index)
	{
		return this->Memory[p_index];
	};
};