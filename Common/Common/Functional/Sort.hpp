#pragma once

#include "Common/Memory/memory_slice.hpp"

template<class ElementType>
struct DefaultSortedElementProvider
{
	inline DefaultSortedElementProvider() {};
	inline ElementType& get(ElementType& p_element)
	{
		return p_element;
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

template<class ComparatorElement, class ComparatorFn>
struct LinearSort
{
	template<class ElementType, class SortedElementProvider>
	inline static size_t get_insertion_index(const ElementType* p_inserted_element, com::MemorySlice<ElementType>& p_array, SortedElementProvider& p_sorted_element_provider)
	{
		size_t p_shaders_indexes_count = p_array.count();

		if (p_shaders_indexes_count == 0)
		{
			return 0;
		}

		ComparatorElement l_inserted_execution_order = p_sorted_element_provider.get(*p_inserted_element);

		for (size_t i = 0; i < p_shaders_indexes_count; i++)
		{
			ComparatorElement l_other = p_sorted_element_provider.get(p_array[i]);
			if (ComparatorFn::compare(l_inserted_execution_order, l_other))
			{
				return i;
			}
		}

		return p_shaders_indexes_count;
	};
};


template<class ElementType>
struct QuickCompare
{
	inline static bool compare(ElementType& p_left, ElementType& p_right);
	inline static bool compare_inv(ElementType& p_left, ElementType& p_right);
};

template<>
struct QuickCompare<size_t>
{
	inline static bool compare(size_t& p_left, size_t& p_right)
	{
		return p_left < p_right;
	};
	inline static bool compare_inv(size_t& p_left, size_t& p_right)
	{
		return p_left > p_right;
	};
};

template<class ComparatorElement, class QuickComparatorFn>
struct QuickpartitionSort
{
	template<class ElementType, class SortedElementProvider>
	inline static void sort_array(com::MemorySlice<ElementType>& p_array, SortedElementProvider& p_sorted_element_provider)
	{
		if (p_array.count() > 0)
		{
			sort_between(p_array, p_array.Begin, p_array.Begin + p_array.count() - 1, p_sorted_element_provider);
		}
	};

	template<class ElementType>
	inline static void sort_array(com::MemorySlice<ElementType>& p_array)
	{
		sort_array(p_array, DefaultSortedElementProvider<ElementType>());
	};
	
private:

	template<class ElementType, class SortedElementProvider>
	inline static void sort_between(com::MemorySlice<ElementType>& p_array, const size_t p_begin, const size_t p_end, SortedElementProvider& p_sorted_element_provider)
	{
		if (p_begin < p_end)
		{ 
			size_t l_pivot = sort_partition(p_array, p_begin, p_end, p_sorted_element_provider);
			sort_between(p_array, p_begin, l_pivot, p_sorted_element_provider);
			sort_between(p_array, l_pivot + 1, p_end, p_sorted_element_provider);
		}
	}

	template<class ElementType, class SortedElementProvider>
	inline static size_t sort_partition(com::MemorySlice<ElementType>& p_array, const size_t p_begin, const size_t p_end, SortedElementProvider& p_sorted_element_provider)
	{
		if (p_array.count() == 0)
		{
			return 0;
		}

		ComparatorElement l_pivot = p_sorted_element_provider.get(p_array[(size_t)floorf(((float)(p_begin + p_end) * 0.5f))]);
		size_t i = p_begin - 1;
		size_t j = p_end + 1;


		while (true)
		{
			do
			{
				i += 1;
			} while (QuickComparatorFn::compare(p_sorted_element_provider.get(p_array[i]), l_pivot));

			do
			{
				j -= 1;
			} while (QuickComparatorFn::compare_inv(p_sorted_element_provider.get(p_array[j]), l_pivot));

			if (i >= j)
			{
				return j;
			}
			ElementType l_tmp = p_array[i];
			p_array[i] = p_array[j];
			p_array[j] = l_tmp;
		}
	};
};
