#pragma once

namespace com
{
	template <class TYPE>
	struct MemorySlice
	{
		TYPE* Memory = nullptr;
		size_t Begin = 0;
		size_t End = 0;

		inline MemorySlice() {};

		inline MemorySlice(TYPE& p_element)
		{
			this->Memory = &p_element;
			this->Begin = 0;
			this->End = 1;
		};
		inline MemorySlice(TYPE* p_element)
		{
			this->Memory = p_element;
			this->Begin = 0;
			this->End = 1;
		};

		inline MemorySlice(TYPE& p_element, const size_t p_element_count)
		{
			this->Memory = &p_element;
			this->Begin = 0;
			this->End = p_element_count;
		};

		inline MemorySlice(TYPE* p_element, const size_t p_element_count)
		{
			this->Memory = p_element;
			this->Begin = 0;
			this->End = p_element_count;
		};

		inline size_t count()
		{
			return this->End - this->Begin;
		};

		inline size_t byte_count()
		{
			return this->count() * sizeof(TYPE);
		};

		inline const TYPE& operator[](size_t p_index) const
		{
			//TODO -> Bound check
			return (this->Memory + Begin)[p_index];
		};

		inline TYPE& operator[](size_t p_index)
		{
			//TODO -> Bound check
			return (this->Memory + Begin)[p_index];
		};
	};

	template<class TYPE, unsigned N>
	struct NMemorySlice
	{
		TYPE Memory[N];

		inline TYPE& operator[](size_t p_index)
		{
			//TODO -> Bound check
			return this->Memory[p_index];
		};

		inline MemorySlice<TYPE> to_memoryslice()
		{
			return MemorySlice<TYPE>(this->Memory, N);
		};

	};
}
