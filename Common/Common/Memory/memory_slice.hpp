#pragma once

namespace com
{
	template <class TYPE>
	struct MemorySlice
	{
		const TYPE* Memory;
		size_t Begin;
		size_t End;

		inline MemorySlice(const TYPE& p_element)
		{
			this->Memory = &p_element;
			this->Begin = 0;
			this->End = 1;
		}

		inline MemorySlice(const TYPE& p_element, const size_t p_element_count)
		{
			this->Memory = &p_element;
			this->Begin = 0;
			this->End = p_element_count;
		}

		inline size_t count()
		{
			return this->End - this->Begin;
		}

		inline size_t byte_count()
		{
			return this->count() * sizeof(TYPE);
		}
	};
}
