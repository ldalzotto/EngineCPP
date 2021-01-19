#pragma once

struct Sort
{
	//TODO ->Implement Quick sort
	template<class ElementType, class CompareFunc>
	inline static void Linear2(Slice<ElementType>& p_slice, const uimax p_start_index)
	{
		for (loop(i, p_start_index, p_slice.Size))
		{
			ElementType& l_left = p_slice.get(i);
			for (loop(j, i, p_slice.Size))
			{
				ElementType& l_right = p_slice.get(j);
				if (CompareFunc::compare(l_left, l_right))
				{
					ElementType l_left_tmp = l_left;
					l_left = l_right;
					l_right = l_left_tmp;
				}
			}
		}
	};
};


#define sort_linear2_begin(ElementType, StructName) \
struct StructName\
{ \
	inline static int8 compare(const ElementType& p_left, const ElementType& p_right)\
	{

#define sort_linear2_end(SliceVariable, ElementType, StructName) \
	};\
};\
auto l_slice_variable_##StructName = (SliceVariable); \
Sort::Linear2<ElementType, StructName>(l_slice_variable_##StructName, 0);

