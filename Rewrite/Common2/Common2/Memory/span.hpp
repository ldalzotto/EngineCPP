#pragma once

/*
    A Span is a heap allocated chunk of memory.
    Span can allocate memory, be resized and freed.
*/
template<class ElementType>
struct Span 
{
    union
    {
        struct
        {
            size_t Capacity;
            ElementType* Memory;
        };
        Slice<ElementType> slice;
    };
};

template<class ElementType>
inline Span<ElementType> span_build(ElementType* p_memory, const size_t p_capacity)
{
	return Span<ElementType>{p_capacity, p_memory};
};

template<class ElementType>
inline Span<ElementType> span_allocate(const size_t p_capacity)
{
    return Span<ElementType>{p_capacity, cast(ElementType*, heap_malloc(p_capacity * sizeof(ElementType)))};
};

template<class ElementType>
inline char span_resize(Span<ElementType>* p_span, const size_t p_new_capacity)
{
    if (p_new_capacity > p_span->Capacity)
    {
        ElementType* l_newMemory = (ElementType*)heap_realloc(cast(char*, p_span->Memory), p_new_capacity * sizeof(ElementType));
        if (l_newMemory != NULL)
        {
            *p_span = span_build(l_newMemory, p_new_capacity);
            return 1;
        }
        return 0;
    }
    return 1;
};

template<class ElementType>
inline void span_resize_until_capacity_met(Span<ElementType>* p_span, const size_t p_desired_capacity)
{
    size_t l_resized_capacity = p_span->Capacity;

    if (l_resized_capacity >= p_desired_capacity)
    {
        return;
    }

    if (l_resized_capacity == 0) { l_resized_capacity = 1; }

    while (l_resized_capacity < p_desired_capacity)
    {
        l_resized_capacity *= 2;
    }

    span_resize(p_span, l_resized_capacity);
};

template<class ElementType>
inline void span_free(Span<ElementType>* p_span)
{
    heap_free(cast(char*, p_span->Memory));
    *p_span = span_build<ElementType>(NULL, 0);
};

template<class ElementType>
inline void span_bound_inside_check(const Span<ElementType>* p_span, const Slice<ElementType>* p_tested_slice)
{
#if CONTAINER_BOUND_TEST
    if ((p_tested_slice->Begin + p_tested_slice->Size) > (p_span->Memory + p_span->Capacity))
    {
        abort();
    }
#endif
};

template<class ElementType>
inline void _span_bound_check(const Span<ElementType>* p_span, const size_t p_index)
{
#if CONTAINER_BOUND_TEST
    if (p_index > p_span->Capacity)
    {
        abort();
    }
#endif
};



template<class ElementType>
inline void span_move_memory_down(Span<ElementType>* p_span, const size_t p_moved_block_size, const size_t p_break_index, const size_t p_move_delta)
{
    Slice<ElementType> l_target = slice_build_memory_offset_elementnb(p_span->Memory, p_break_index + p_move_delta, p_moved_block_size - p_break_index);
#if CONTAINER_BOUND_TEST
    span_bound_inside_check(p_span, &l_target);
#endif		
    Slice<ElementType> l_source = slice_build(p_span->Memory, p_break_index, p_moved_block_size);
    slice_memmove(&l_target, &l_source);
};

template<class ElementType>
inline void span_move_memory_up(Span<ElementType>* p_span, const size_t p_moved_block_size, const size_t p_break_index, const size_t p_move_delta)
{
    Slice<ElementType> l_target = slice_build_memory_offset_elementnb(p_span->Memory, p_break_index, p_moved_block_size - p_break_index);
#if CONTAINER_BOUND_TEST
    span_bound_inside_check(p_span, &l_target);
#endif		
    Slice<ElementType> l_source = slice_build(p_span->Memory, p_break_index + p_move_delta, p_moved_block_size);
    slice_memmove(&l_target, &l_source);
};

template<class ElementType>
inline void span_copy_memory(Span<ElementType>* p_span, const size_t p_copy_index, const Slice<ElementType>* p_elements)
{
    Slice<ElementType> l_target = slice_build_memory_elementnb(p_span->Memory + p_copy_index, p_elements->Size);

#if CONTAINER_BOUND_TEST
    span_bound_inside_check(p_span, &l_target);
#endif

    slice_memcpy(
        &l_target,
        p_elements
    );
};

template<class ElementType>
inline void span_copy_memory_2v(Span<ElementType>* p_span, const size_t p_copy_index, const Slice<ElementType> p_elements)
{
    span_copy_memory<ElementType>(p_span, p_copy_index, &p_elements);
};