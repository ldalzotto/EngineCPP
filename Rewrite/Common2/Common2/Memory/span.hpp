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

    inline static Span<ElementType> build(ElementType* p_memory, const size_t p_capacity)
    {
        return Span<ElementType>{p_capacity, p_memory};
    };

    inline static Span<ElementType> allocate(const size_t p_capacity)
    {
        return Span<ElementType>{p_capacity, cast(ElementType*, heap_malloc(p_capacity * sizeof(ElementType)))};
    };


    inline char resize(const size_t p_new_capacity)
    {
        if (p_new_capacity > this->Capacity)
        {
            ElementType* l_newMemory = (ElementType*)heap_realloc(cast(char*, this->Memory), p_new_capacity * sizeof(ElementType));
            if (l_newMemory != NULL)
            {
                *this = Span<ElementType>::build(l_newMemory, p_new_capacity);
                return 1;
            }
            return 0;
        }
        return 1;
    };

    inline void resize_until_capacity_met(const size_t p_desired_capacity)
    {
        size_t l_resized_capacity = this->Capacity;

        if (l_resized_capacity >= p_desired_capacity)
        {
            return;
        }

        if (l_resized_capacity == 0) { l_resized_capacity = 1; }

        while (l_resized_capacity < p_desired_capacity)
        {
            l_resized_capacity *= 2;
        }

        this->resize(l_resized_capacity);
    };

    inline void free()
    {
        heap_free(cast(char*, this->Memory));
        *this = Span<ElementType>::build(NULL, 0);
    };

    inline void bound_inside_check(const Slice<ElementType>* p_tested_slice)
    {
#if CONTAINER_BOUND_TEST
        if ((p_tested_slice->Begin + p_tested_slice->Size) > (this->Memory + this->Capacity))
        {
            abort();
        }
#endif
    };


    inline void bound_check(const size_t p_index)
    {
#if CONTAINER_BOUND_TEST
        if (p_index > this->Capacity)
        {
            abort();
        }
#endif
    };



    inline void move_memory_down(const size_t p_moved_block_size, const size_t p_break_index, const size_t p_move_delta)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Memory, p_break_index + p_move_delta, p_moved_block_size - p_break_index);
#if CONTAINER_BOUND_TEST
        this->bound_inside_check(&l_target);
#endif		
        Slice<ElementType> l_source = Slice<ElementType>::build(this->Memory, p_break_index, p_moved_block_size);
        slice_memmove(&l_target, &l_source);
    };

    inline void move_memory_up(const size_t p_moved_block_size, const size_t p_break_index, const size_t p_move_delta)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_offset_elementnb(this->Memory, p_break_index, p_moved_block_size - p_break_index);
#if CONTAINER_BOUND_TEST
        this->bound_inside_check(&l_target);
#endif		
        Slice<ElementType> l_source = Slice<ElementType>::build(this->Memory, p_break_index + p_move_delta, p_moved_block_size);
        slice_memmove(&l_target, &l_source);
    };

    inline void copy_memory(const size_t p_copy_index, const Slice<ElementType>* p_elements)
    {
        Slice<ElementType> l_target = Slice<ElementType>::build_memory_elementnb(this->Memory + p_copy_index, p_elements->Size);

#if CONTAINER_BOUND_TEST
        this->bound_inside_check(&l_target);
#endif

        slice_memcpy(
            &l_target,
            p_elements
        );
    };

    inline void copy_memory_2v(const size_t p_copy_index, const Slice<ElementType> p_elements)
    {
        this->copy_memory(p_copy_index, &p_elements);
    };
};



