#pragma once

/*
	//TODO

	functions missing :
		- insert array version of vectorofvector_element_insert_element_at and vectorofvector_element_erase_element_at
*/

/*
	A VectorOfVector is a chain of resizable Vector allocated on the same memory block.
	Every nested vectors can be altered with "vectorofvector_element_*" functions.
*/
template<class ElementType>
struct VectorOfVector
{
	VaryingVector varying_vector;
};

/*
template<class ElementType>
struct VectorOfVector
{
	VaryingVector varying_vector;
};
*/
/*
	The header of every vector of the VectorOfVector. That indicates the associated memory chunk state.
*/
struct VectorOfVector_VectorHeader
{
	size_t Size;
	size_t Capacity;
};

/*
	Interface returned when a nested vector is requested.
	We can use the Memory field to iterate over the nested vector.
*/
template<class ElementType>
struct VectorOfVector_Element
{
	VectorOfVector_VectorHeader Header;
	Slice<ElementType> Memory;
};

inline VectorOfVector_VectorHeader _vectorofvector_vectorheader_build(const size_t p_size, const size_t p_capacity)
{
	return VectorOfVector_VectorHeader{ p_size, p_capacity };
};

inline VectorOfVector_VectorHeader _vectorofvector_vectorheader_build_default()
{
	return _vectorofvector_vectorheader_build(0, 0);
};

inline Span<char> _vectorofvector_vectorheader_allocate(const Slice<char>* p_vector_slice, const size_t p_vector_size)
{
	Span<char> l_allocated_element = span_allocate<char>(sizeof(VectorOfVector_VectorHeader) + p_vector_slice->Size);
	VectorOfVector_VectorHeader l_vector_header = _vectorofvector_vectorheader_build(p_vector_size, p_vector_size);
	span_copy_memory_2v(&l_allocated_element, 0, slice_build_aschar_memory_elementnb(&l_vector_header, 1));
	span_copy_memory(&l_allocated_element, sizeof(l_vector_header), p_vector_slice);
	return l_allocated_element;
};

inline Span<char> _vectorofvector_vectorheader_allocate_0v(const Slice<char> p_vector_slice, const size_t p_vector_size)
{
	return _vectorofvector_vectorheader_allocate(&p_vector_slice, p_vector_size);
};

template<class ElementType>
inline Span<char> _vectorofvector_vectorheader_allocate_vectorelements(const Slice<ElementType>* p_vector_elements)
{
	return _vectorofvector_vectorheader_allocate_0v(slice_build_aschar_slice(p_vector_elements), p_vector_elements->Size);
};

inline size_t _vectorofvector_vectorheader_get_vector_offset()
{
	return sizeof(VectorOfVector_VectorHeader);
};

inline size_t _vectorofvector_vectorheader_get_vector_element_offset(const size_t p_element_index, const size_t p_element_size)
{
	return _vectorofvector_vectorheader_get_vector_offset() + (p_element_index * p_element_size);
};

template<class ElementType>
inline Slice<ElementType> _vectorofvector_vectorheader_get_vector_to_capacity(VectorOfVector_VectorHeader* p_header)
{
	return slice_build_memory_elementnb(cast(ElementType*, cast(char*, p_header) + sizeof(VectorOfVector_VectorHeader)), p_header->Capacity);
};

template<class ElementType>
inline VectorOfVector<ElementType> vectorofvector_allocate_default()
{
	return castv(VectorOfVector<ElementType>, varyingvector_allocate_default());
};

template<class ElementType>
inline void vectorofvector_free(VectorOfVector<ElementType>* p_vector_of_vector)
{
	varyingvector_free(&p_vector_of_vector->varying_vector);
};

template<class ElementType>
inline void vectorofvector_push_back(VectorOfVector<ElementType>* p_vector_of_vector)
{
	VectorOfVector_VectorHeader l_header = _vectorofvector_vectorheader_build_default();
	Slice<char> l_header_slice = slice_build_aschar_memory_elementnb(&l_header, 1);
	varyingvector_push_back(&p_vector_of_vector->varying_vector, &l_header_slice);
};

template<class ElementType>
inline void vectorofvector_push_back_element(VectorOfVector<ElementType>* p_vector_of_vector, const Slice<ElementType>* p_vector_elements)
{
	Span<char> l_pushed_memory = _vectorofvector_vectorheader_allocate_vectorelements(p_vector_elements);
	varyingvector_push_back(&p_vector_of_vector->varying_vector, &l_pushed_memory.slice);
	span_free(&l_pushed_memory);
};


template<class ElementType>
inline void vectorofvector_insert_at(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_index)
{
	VectorOfVector_VectorHeader l_header = _vectorofvector_vectorheader_build_default();
	Slice<char> l_header_slice = slice_build_aschar_memory_elementnb(&l_header, 1);
	varyingvector_insert_at(p_vector_of_vector, &l_header_slice, p_index);
};


template<class ElementType>
inline VectorOfVector_Element<ElementType> vectorofvector_get(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_index)
{
	Slice<char> l_element = varyingvector_get(&p_vector_of_vector->varying_vector, p_index);
	return VectorOfVector_Element<ElementType>{
		*(cast(VectorOfVector_VectorHeader*, l_element.Begin)),
			slice_cast_0v<ElementType>(slice_slide_rv(&l_element, _vectorofvector_vectorheader_get_vector_offset()))
	};
};

template<class ElementType>
inline Slice<ElementType> vectorofvector_get_vector_memory(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_index)
{
	return slice_cast_0v<ElementType>(slice_slide_rv0v(varyingvector_get(p_vector_of_vector, p_index), _vectorofvector_vectorheader_get_vector_offset()));
};

template<class ElementType>
inline VectorOfVector_VectorHeader* vectorofvector_get_vectorheader(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_index)
{
	return cast(VectorOfVector_VectorHeader*, varyingvector_get(&p_vector_of_vector->varying_vector, p_index).Begin);
};


template<class ElementType>
inline void _vectorofvector_element_movememory_up(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, VectorOfVector_VectorHeader* p_nested_vector_header,
	const size_t p_break_index, const size_t p_move_delta)
{
	Slice<char> l_source =
		slice_build_aschar_slice_0v(
			slice_slide_rv0v(
				_vectorofvector_vectorheader_get_vector_to_capacity<ElementType>(p_nested_vector_header),
				p_break_index
			)
		);

	varyingvector_element_movememory(
		&p_vector_of_vector->varying_vector,
		p_nested_vector_index,
		_vectorofvector_vectorheader_get_vector_element_offset(p_break_index - p_move_delta, sizeof(ElementType)),
		&l_source
	);
};

template<class ElementType>
inline void _vectorofvector_element_movememory_down(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, VectorOfVector_VectorHeader* p_nested_vector_header,
	const size_t p_break_index, const size_t p_move_delta)
{
	Slice<char> l_source =
		slice_build_aschar_slice_0v(
			slice_slide_rv0v(
				_vectorofvector_vectorheader_get_vector_to_capacity<ElementType>(p_nested_vector_header),
				p_break_index
			)
		);

	varyingvector_element_movememory(
		&p_vector_of_vector->varying_vector,
		p_nested_vector_index,
		_vectorofvector_vectorheader_get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)),
		&l_source
	);
};

template<class ElementType>
inline void _vectorofvector_element_movememory_down_and_resize(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, VectorOfVector_VectorHeader* p_nested_vector_header,
	const size_t p_break_index, const size_t p_move_delta)
{
	Slice<char> l_source =
		slice_build_aschar_slice_0v(
			slice_slide_rv0v(
				_vectorofvector_vectorheader_get_vector_to_capacity<ElementType>(p_nested_vector_header),
				p_break_index
			)
		);

	varyingvector_element_expand(&p_vector_of_vector->varying_vector, p_nested_vector_index, _vectorofvector_vectorheader_get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)) + l_source.Size);

	varyingvector_element_movememory(
		&p_vector_of_vector->varying_vector,
		p_nested_vector_index,
		_vectorofvector_vectorheader_get_vector_element_offset(p_break_index + p_move_delta, sizeof(ElementType)),
		&l_source
	);
};

template<class ElementType>
inline void _vectorofvector_element_write_element(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const size_t p_write_start_index, const ElementType* p_element)
{
	varyingvector_element_writeto_3v(
		&p_vector_of_vector->varying_vector,
		p_nested_vector_index,
		_vectorofvector_vectorheader_get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
		slice_build_aschar_memory_singleelement(p_element)
	);
};

template<class ElementType>
inline void _vectorofvector_element_write_array(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const size_t p_write_start_index, const Slice<ElementType>* p_elements)
{
	varyingvector_element_writeto_3v(
		&p_vector_of_vector->varying_vector,
		p_nested_vector_index,
		_vectorofvector_vectorheader_get_vector_element_offset(p_write_start_index, sizeof(ElementType)),
		slice_build_aschar_memory_elementnb(p_elements->Begin, p_elements->Size)
	);
};

template<class ElementType>
inline void _vectorofvector_element_write_array_3v(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const size_t p_write_start_index, const Slice<ElementType> p_elements)
{
	_vectorofvector_element_write_array(p_vector_of_vector, p_nested_vector_index, p_write_start_index, &p_elements);
};

template<class ElementType>
inline void vectorofvector_element_push_back_element(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const ElementType* p_element)
{
	VectorOfVector_VectorHeader* l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);

	if (l_vector_header->Size + 1 > l_vector_header->Capacity)
	{
		varyingvector_element_expand_with_value_2v(
			&p_vector_of_vector->varying_vector,
			p_nested_vector_index,
			slice_build_aschar_memory_elementnb(p_element, 1)
		);

		// /!\ because we potentially reallocate the p_vector_of_vector, we nee to requery for the VectorOfVector_VectorHeader
		l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);
		l_vector_header->Capacity += 1;
	}
	else
	{
		_vectorofvector_element_write_element(p_vector_of_vector, p_nested_vector_index, l_vector_header->Size, p_element);
	}

	l_vector_header->Size += 1;
};


template<class ElementType>
inline void vectorofvector_element_push_back_array(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const Slice<ElementType>* p_elements)
{
	VectorOfVector_VectorHeader* l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);

	if (l_vector_header->Size == l_vector_header->Capacity)
	{
		varyingvector_element_expand_with_value_2v(
			&p_vector_of_vector->varying_vector,
			p_nested_vector_index,
			slice_build_aschar_slice(p_elements)
		);

		l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);
		l_vector_header->Size += p_elements->Size;
		l_vector_header->Capacity += p_elements->Size;
	}
	else if (l_vector_header->Size + p_elements->Size > l_vector_header->Capacity)
	{
		size_t l_write_element_nb = l_vector_header->Capacity - l_vector_header->Size;
		size_t l_expand_element_nb = p_elements->Size - l_write_element_nb;
		
		_vectorofvector_element_write_array_3v(p_vector_of_vector, p_nested_vector_index, l_vector_header->Size, slice_build_memory_elementnb(p_elements->Begin, l_write_element_nb));

		varyingvector_element_expand_with_value_2v(
			&p_vector_of_vector->varying_vector,
			p_nested_vector_index,
			slice_build_aschar_memory_elementnb(p_elements->Begin + l_write_element_nb, l_expand_element_nb)
		);

		l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);
		l_vector_header->Size += p_elements->Size;
		l_vector_header->Capacity += l_expand_element_nb;
	}
	else
	{
		_vectorofvector_element_write_array(p_vector_of_vector, p_nested_vector_index, l_vector_header->Size, p_elements);

		l_vector_header->Size += p_elements->Size;
	}


};

template<class ElementType>
inline void vectorofvector_element_insert_element_at(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const size_t p_index, const ElementType* p_element)
{
	VectorOfVector_VectorHeader* l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);

#if CONTAINER_BOUND_TEST
	assert_true(p_index != l_vector_header->Size); //use vectorofvector_element_push_back_element
	assert_true(p_index < l_vector_header->Size);
#endif

	if ((l_vector_header->Size + 1) > l_vector_header->Capacity)
	{
		_vectorofvector_element_movememory_down_and_resize<ElementType>(p_vector_of_vector, p_nested_vector_index, l_vector_header, p_index, 1);
		l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);
	}
	else
	{
		_vectorofvector_element_movememory_down<ElementType>(p_vector_of_vector, p_nested_vector_index, l_vector_header, p_index, 1);
	}

	l_vector_header->Size += 1;

	_vectorofvector_element_write_element(p_vector_of_vector, p_nested_vector_index, p_index, p_element);
}

template<class ElementType>
inline void vectorofvector_element_erase_element_at(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index, const size_t p_index)
{
	VectorOfVector_VectorHeader* l_vector_header = vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index);

#if CONTAINER_BOUND_TEST
	if (p_index == l_vector_header->Size)
	{
		abort(); //use vectorofvector_element_pop_back_element
	}
#endif

	_vectorofvector_element_movememory_up<ElementType>(p_vector_of_vector, p_nested_vector_index, l_vector_header, p_index + 1, 1);

	l_vector_header->Size -= 1;
};

template<class ElementType>
inline void vectorofvector_element_clear(VectorOfVector<ElementType>* p_vector_of_vector, const size_t p_nested_vector_index)
{
	vectorofvector_get_vectorheader<ElementType>(p_vector_of_vector, p_nested_vector_index)->Size = 0;
}


/*
template<class ElementType>
inline VectorOfVector<ElementType>* varyingvector_cast_vectorofvector(VaryingVector* p_varying_vector)
{
	return cast(VectorOfVector<ElementType>*, p_varying_vector);
};
*/
