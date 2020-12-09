#pragma once

#include "vector_def.hpp"
#include <stdlib.h>
#include <string.h>

namespace com
{

#define Vector_TemplateHeader template<class TYPE, class Allocator>
#define Vector_ClassName Vector<TYPE, Allocator>

	template <class TYPE>
	inline size_t getElementOffset(const size_t p_index)
	{
		return sizeof(TYPE) * p_index;
	};

	Vector_TemplateHeader
		inline void copy(Vector_ClassName* thiz, const Vector_ClassName& p_other)
	{
		if (thiz != &p_other)
		{
			thiz->allocator = p_other.allocator;
			thiz->Memory = (TYPE*)thiz->allocator.malloc(p_other.Capacity * sizeof(TYPE));
			memcpy(thiz->Memory, p_other.Memory, p_other.Capacity * sizeof(TYPE));


			thiz->Capacity = p_other.Capacity;
			thiz->Size = p_other.Size;
		}
	};

	Vector_TemplateHeader
		inline Vector_ClassName::Vector()
	{
		this->Memory = nullptr;
		this->Capacity = 0;
		this->Size = 0;
	}

	Vector_TemplateHeader
		inline TYPE& Vector_ClassName::operator[](size_t i)
	{
#if CONTAINER_BOUND_TEST
		if (i >= this->Size)
		{
			abort();
		}
#endif
		return this->Memory[i];
	};

	Vector_TemplateHeader inline void Vector_ClassName::allocate(size_t p_initialSize, const Allocator& p_allocator)
	{
		this->allocator = p_allocator;
		this->Memory = (TYPE*)this->allocator.malloc(p_initialSize * sizeof(TYPE));
		this->Capacity = p_initialSize;
		this->Size = 0;
	}

	Vector_TemplateHeader inline void Vector_ClassName::free()
	{
		this->allocator.free(this->Memory);
		this->Memory = nullptr;
		this->Capacity = 0;
		this->Size = 0;
	}

	Vector_TemplateHeader inline void Vector_ClassName::clear()
	{
		this->Size = 0;
	}

	Vector_TemplateHeader
		inline size_t Vector_ClassName::size_in_bytes() const
	{
		return this->Size * sizeof(TYPE);
	};

	Vector_TemplateHeader
		inline size_t Vector_ClassName::capacity_in_bytes()
	{
		return this->Capacity * sizeof(TYPE);
	};

	Vector_TemplateHeader
		inline char Vector_ClassName::resize(const size_t p_newCapacity)
	{
		if (p_newCapacity > this->Capacity)
		{
			TYPE* l_newMemory = (TYPE*)this->allocator.realloc(this->Memory, p_newCapacity * sizeof(TYPE));
			if (l_newMemory != NULL)
			{
				this->Memory = l_newMemory;
				this->Capacity = p_newCapacity;
				return 1;
			}
			return 0;
		}

		return 1;
	}

	Vector_TemplateHeader inline char Vector_ClassName::push_back(const TYPE& p_element)
	{
		if (this->Size >= this->Capacity)
		{
			if (!this->resize(this->Capacity == 0 ? 1 : (this->Capacity * 2)))
			{
				return 0;
			};
			return this->push_back(p_element);
		}
		else
		{
			void* p_targetMemory = (char*)this->Memory + getElementOffset<TYPE>(this->Size);
			memcpy(p_targetMemory, &p_element, sizeof(TYPE));
			this->Size += 1;
		}

		return 1;
	}

	Vector_TemplateHeader inline char Vector_ClassName::push_back(MemorySlice<TYPE>& p_elements)
	{
		return this->insert_at(p_elements, this->Size - 1);
	};

	Vector_TemplateHeader
		inline char Vector_ClassName::insert_at(MemorySlice<TYPE>& p_elements, const size_t p_index)
	{
		if (p_index > this->Size)
		{
			return 1;
		}

		if (this->Size + p_elements.count() > this->Capacity)
		{
			this->resize(this->Capacity == 0 ? 1 : (this->Capacity * 2));
			this->insert_at(p_elements, p_index);
		}
		else
		{
			void* l_initialElement = (char*)this->Memory + getElementOffset<TYPE>(p_index);
			// If we insert between existing elements, we move down memory to give space for new elements
			if (this->Size > p_index)
			{
				void* l_targetElement = (char*)this->Memory + getElementOffset<TYPE>(p_index + p_elements.count());
				memmove(l_targetElement, l_initialElement, sizeof(TYPE) * (this->Size - p_index));
			}
			memcpy(l_initialElement, p_elements.Memory + p_elements.Begin, p_elements.byte_count());
			this->Size += p_elements.count();
		}

		return 0;
	};

	Vector_TemplateHeader
		inline char Vector_ClassName::insert_at(const TYPE& p_element, const size_t p_index)
	{
		return this->insert_at(MemorySlice<TYPE>(p_element), p_index);
	};

	Vector_TemplateHeader
		template<class SortFn, class ComparatorElementsProvider>
	inline char Vector_ClassName::insert_at_v2(const TYPE& p_element, ComparatorElementsProvider& p_element_provider)
	{
		return this->insert_at(p_element, SortFn::get_insertion_index(&p_element, this->to_memoryslice(), p_element_provider));
	};

	Vector_TemplateHeader
		inline char Vector_ClassName::erase_at(const size_t p_index, const size_t p_size)
	{
		if (p_index >= this->Size)
		{
			return 1;
		}

		// If we are not erasing the last element, then we move memory. Else, we have nothing to do.
		if (p_index + p_size != this->Size)
		{
			void* p_targetMemory = (char*)this->Memory + getElementOffset<TYPE>(p_index);
			memmove(p_targetMemory, (char*)p_targetMemory + (p_size * sizeof(TYPE)), (this->Size - p_index - p_size) * sizeof(TYPE));
		}

		this->Size -= p_size;
		return 0;
	};

	Vector_TemplateHeader
		inline char Vector_ClassName::swap(const size_t p_left, const size_t p_right)
	{
		if (p_left == p_right || p_left >= this->Size || p_right >= this->Size) { return 0; }

		TYPE* l_leftMemoryTarget = (TYPE*)((char*)this->Memory + getElementOffset<TYPE>(p_left));
		TYPE* l_rightMemoryTarget = (TYPE*)((char*)this->Memory + getElementOffset<TYPE>(p_right));

		TYPE l_rightTmp = *l_rightMemoryTarget;
		*l_rightMemoryTarget = *l_leftMemoryTarget;
		*l_leftMemoryTarget = l_rightTmp;

		return 0;
	};

	Vector_TemplateHeader
		inline Vector<TYPE, Allocator> Vector_ClassName::clone() const
	{
		Vector<TYPE, Allocator> l_copy;
		copy(&l_copy, *this);
		return l_copy;
	};

	Vector_TemplateHeader
		inline Vector<TYPE, Allocator> Vector_ClassName::move()
	{
		Vector<TYPE, Allocator> l_target = *this;
		this->Memory = nullptr;
		this->Capacity = 0;
		this->Size = 0;
		return l_target;
	};

	Vector_TemplateHeader
		inline MemorySlice<TYPE> Vector_ClassName::to_memoryslice()
	{
		return MemorySlice<TYPE>(*this->Memory, this->Size);
	};
}
