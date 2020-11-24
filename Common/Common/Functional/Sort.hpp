#pragma once

#include "Common/Memory/memory_slice.hpp"

template<class ComparatorElement, class ComparatorFn>
struct LinearSort
{
	template<class ElementType, class ComparatorElementsProvider>
	inline static size_t get_insertion_index(const ElementType* p_inserted_element, com::MemorySlice<ElementType>& p_array, ComparatorElementsProvider& p_comparator_element_provider)
	{
		size_t p_shaders_indexes_count = p_array.count();

		if (p_shaders_indexes_count == 0)
		{
			return 0;
		}

		ComparatorElement l_inserted_execution_order = p_comparator_element_provider.get(*p_inserted_element);

		for (size_t i = 0; i < p_shaders_indexes_count; i++)
		{
			ComparatorElement l_other = p_comparator_element_provider.get(p_array[i]);
			if (ComparatorFn::compare(l_inserted_execution_order, l_other))
			{
				return i;
			}
		}

		return p_shaders_indexes_count;
	};
};


template<class ElementType>
struct Compare
{
	inline static bool compare(ElementType& p_left, ElementType& p_right);
};

template<>
struct Compare<size_t>
{
	inline static bool compare(size_t& p_left, size_t& p_right)
	{
		return p_left <= p_right;
	};
};