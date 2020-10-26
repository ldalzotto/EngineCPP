#pragma once

template <class TYPE>
struct Array
{
	TYPE* Memory;
	size_t Capacity;

	inline Array(TYPE* p_memory, const size_t p_capacity) : Memory{p_memory}, Capacity{p_capacity} { }
	inline dispose() { free(this->Memory); this->Capacity = 0; }
};