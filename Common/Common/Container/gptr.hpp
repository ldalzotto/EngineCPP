#pragma once

struct GPtr
{
	size_t element_size = 0;
	char* ptr = nullptr;

	inline GPtr(char* p_ptr, const size_t p_element_size)
	{
		this->ptr = p_ptr;
		this->element_size = p_element_size;
	};

	inline GPtr()
	{

	};

	template<class ElementType>
	inline static GPtr fromType(ElementType* p_ptr)
	{
		return GPtr((char*)p_ptr, sizeof(ElementType));
	};

	template<class ElementType>
	inline static const GPtr fromType(const ElementType* p_ptr)
	{
		return GPtr((char*)p_ptr, sizeof(ElementType));
	};
};