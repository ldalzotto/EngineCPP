#pragma once

struct Sort
{
	//TODO ->Implement Quick sort
	template<class ElementType, class CompareFunc>
	inline static void Linear2(Slice<ElementType>* p_slice, const size_t p_start_index)
	{
		for (size_t i = p_start_index; i < p_slice->Size; i++)
		{
			ElementType* l_left = p_slice->get(i);
			for (size_t j = i; j < p_slice->Size; j++)
			{
				ElementType* l_right = p_slice->get(j);
				if (CompareFunc::compare(l_left, l_right))
				{
					ElementType l_left_tmp = *l_left;
					*l_left = *l_right;
					*l_right = l_left_tmp;
				}
			}
		}
	};

	template<class ElementType, class CompareFunc>
	inline static void Linear2(Slice<ElementType> p_slice, const size_t p_start_index)
	{
		return Linear2<ElementType, CompareFunc>(&p_slice, p_start_index);
	};
};


#define sort_linear2_begin(ElementType, StructName) \
struct StructName\
{ \
	inline static char compare(const ElementType* p_left, const ElementType* p_right)\
	{

#define sort_linear2_end(SliceVariable, ElementType, StructName) \
	};\
};\
Sort::Linear2<ElementType, StructName>((SliceVariable), 0);

