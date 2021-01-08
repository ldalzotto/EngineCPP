#pragma once

/*
	A Vector is a Span with an imaginary boundary (Size).
	Vector memory is continuous, there is no "hole" between items.
	Vector is reallocated on need.
	Any memory access outside of this imaginary boundary will be in error.
	The Vector expose some safe way to insert/erase data (array or single element).
*/
template<class ElementType>
struct Vector
{
	size_t Size;
	Span<ElementType> Span;
};


template<class ElementType>
inline void _vector_bound_check(Vector<ElementType>* p_vector, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	if (p_index > p_vector->Size)
	{
		abort();
	}
#endif
};


template<class ElementType>
inline void _vector_bound_head_check(Vector<ElementType>* p_vector, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	if (p_index == p_vector->Size)
	{
		abort();
	}
#endif
};


template<class ElementType>
inline Vector<ElementType> vector_build(ElementType* p_memory, size_t p_initial_capacity)
{
	return Vector<ElementType>{0, span_build(p_memory, p_initial_capacity)};
};

template<class ElementType>
inline Vector<ElementType> vector_allocate(const size_t p_initial_capacity)
{
	return Vector<ElementType>{0, span_allocate<ElementType>(p_initial_capacity)};
};



template<class ElementType>
inline void vector_free(Vector<ElementType>* p_vector)
{
	span_free(&p_vector->Span);
	*p_vector = vector_build<ElementType>(NULL, 0);
};

template<class ElementType>
inline ElementType* vector_get_memory(Vector<ElementType>* p_vector)
{
	return p_vector->Span.Memory;
};

template<class ElementType>
inline size_t vector_get_capacity(Vector<ElementType>* p_vector)
{
	return p_vector->Span.Capacity;
};

template<class ElementType>
inline char vector_empty(Vector<ElementType>* p_vector)
{
	return p_vector->Size == 0;
};

template<class ElementType>
inline ElementType* vector_get(Vector<ElementType>* p_vector, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_head_check(p_vector, p_index);
#endif
	return &p_vector->Span.Memory[p_index];
};

template<class ElementType>
inline ElementType vector_get_rv(Vector<ElementType>* p_vector, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_head_check(p_vector, p_index);
#endif
	return p_vector->Span.Memory[p_index];
};


template<class ElementType>
inline void vector_clear(Vector<ElementType>* p_vector)
{
	p_vector->Size = 0;
};

template<class ElementType>
inline void _vector_move_memory_down(Vector<ElementType>* p_vector, const size_t p_break_index, const size_t p_move_delta)
{
	span_move_memory_down(&p_vector->Span, p_vector->Size, p_break_index, p_move_delta);
};

template<class ElementType>
inline void _vector_move_memory_up(Vector<ElementType>* p_vector, const size_t p_break_index, const size_t p_move_delta)
{
	span_move_memory_up(&p_vector->Span, p_vector->Size, p_break_index, p_move_delta);
};

template<class ElementType>
inline char _vector_insert_array_at_unchecked(Vector<ElementType>* p_vector, const Slice<ElementType>* p_elements, const size_t p_index)
{
	span_resize_until_capacity_met(&p_vector->Span, p_vector->Size + p_elements->Size);
	_vector_move_memory_down(p_vector, p_index, p_elements->Size);
	span_copy_memory(&p_vector->Span, p_index, p_elements);

	p_vector->Size += p_elements->Size;

	return 1;
};

template<class ElementType>
inline char vector_insert_array_at(Vector<ElementType>* p_vector, const Slice<ElementType>* p_elements, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_head_check(p_vector, p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

	return _vector_insert_array_at_unchecked(p_vector, p_elements, p_index);
};

template<class ElementType>
inline char vector_insert_array_at_1v(Vector<ElementType>* p_vector, const Slice<ElementType> p_elements, const size_t p_index)
{
	return vector_insert_array_at(p_vector, &p_elements, p_index);
}

template<class ElementType>
inline char _vector_insert_element_at_unchecked(Vector<ElementType>* p_vector, const ElementType* p_element, const size_t p_index)
{
	span_resize_until_capacity_met(&p_vector->Span, p_vector->Size + 1);
	_vector_move_memory_down(p_vector, p_index, 1);
	p_vector->Span.Memory[p_index] = *p_element;
	p_vector->Size += 1;

	return 1;
};

template<class ElementType>
inline char vector_insert_element_at(Vector<ElementType>* p_vector, const ElementType* p_element, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_head_check(p_vector, p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

	return _vector_insert_element_at_unchecked(p_vector, p_element, p_index);
};

template<class ElementType>
inline char vector_insert_element_at_1v(Vector<ElementType>* p_vector, const ElementType p_element, const size_t p_index)
{
	return vector_insert_element_at(p_vector, &p_element, p_index);
};

template<class ElementType>
inline char vector_push_back_array(Vector<ElementType>* p_vector, const Slice<ElementType>* p_elements)
{
	span_resize_until_capacity_met(&p_vector->Span, p_vector->Size + p_elements->Size);
	span_copy_memory(&p_vector->Span, p_vector->Size, p_elements);
	p_vector->Size += p_elements->Size;

	return 1;
};

template<class ElementType>
inline char vector_push_back_array_1v(Vector<ElementType>* p_vector, const Slice<ElementType> p_elements)
{
	return vector_push_back_array(p_vector, &p_elements);
};

template<class ElementType>
inline char vector_push_back_element_empty(Vector<ElementType>* p_vector)
{
	span_resize_until_capacity_met(&p_vector->Span, p_vector->Size + 1);
	p_vector->Size += 1;
	return 1;
};

template<class ElementType>
inline char vector_push_back_element(Vector<ElementType>* p_vector, const ElementType* p_element)
{
	span_resize_until_capacity_met(&p_vector->Span, p_vector->Size + 1);
	p_vector->Span.Memory[p_vector->Size] = *p_element;
	p_vector->Size += 1;

	return 1;
};

template<class ElementType>
inline char vector_push_back_element_1v(Vector<ElementType>* p_vector, const ElementType p_element)
{
	return vector_push_back_element(p_vector, &p_element);
};



template<class ElementType>
inline char vector_insert_array_at_always(Vector<ElementType>* p_vector, const Slice<ElementType>* p_elements, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
#endif
	if (p_index == p_vector->Size)
	{
		return vector_push_back_array(p_vector, p_elements);
	}
	else
	{
		return _vector_insert_array_at_unchecked(p_vector, p_elements, p_index);
	}
};


template<class ElementType>
inline char vector_insert_element_at_always(Vector<ElementType>* p_vector, const ElementType* p_element, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
#endif

	if (p_index == p_vector->Size)
	{
		return vector_push_back_element(p_vector, p_element);
	}
	else
	{
		return _vector_insert_element_at_unchecked(p_vector, p_element, p_index);
	}
};


template<class ElementType>
inline char vector_erase_array_at(Vector<ElementType>* p_vector, const size_t p_index, const size_t p_element_nb)
{

#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_check(p_vector, p_index + p_element_nb);
	_vector_bound_head_check(p_vector, p_index); // use vector_pop_back_array //TODO -> create a "always" variant of vector_erase_array_at
#endif

	_vector_move_memory_up(p_vector, p_index, p_element_nb);
	p_vector->Size -= p_element_nb;

	return 1;
};

template<class ElementType>
inline char vector_erase_element_at(Vector<ElementType>* p_vector, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
	_vector_bound_check(p_vector, p_index);
	_vector_bound_head_check(p_vector, p_index); // use vector_pop_back //TODO -> create a "always" variant of vector_erase_element_at
#endif

	_vector_move_memory_up(p_vector, p_index, 1);
	p_vector->Size -= 1;

	return 1;
};


template<class ElementType>
inline char vector_pop_back_array(Vector<ElementType>* p_vector, const size_t p_element_nb)
{
	p_vector->Size -= p_element_nb;
	return 1;
};

template<class ElementType>
inline char vector_pop_back(Vector<ElementType>* p_vector)
{
	p_vector->Size -= 1;
	return 1;
};


#define vector_loop(VectorVariable, Iteratorname) size_t Iteratorname = 0; Iteratorname < (VectorVariable)->Size; Iteratorname++
#define vector_loop_reverse(VectorVariable, Iteratorname) size_t Iteratorname = (VectorVariable)->Size - 1; Iteratorname != -1; --Iteratorname

