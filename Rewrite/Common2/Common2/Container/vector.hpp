#pragma once

//TODO -> delete when common2 migration is complete
namespace v2
{

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
		Span<ElementType> Memory;

		inline static Vector<ElementType> build(ElementType* p_memory, size_t p_initial_capacity)
		{
			return Vector<ElementType>{0, Span<ElementType>::build(p_memory, p_initial_capacity)};
		};

		inline static Vector<ElementType> allocate(const size_t p_initial_capacity)
		{
			return Vector<ElementType>{0, Span<ElementType>::allocate(p_initial_capacity)};
		};

		inline Slice<ElementType> to_slice()
		{
			return Slice<ElementType>::build_memory_elementnb(this->Memory.Memory, this->Size);
		};

		inline void free()
		{
			this->Memory.free();
			*this = Vector<ElementType>::build(NULL, 0);
		};

		inline ElementType* get_memory()
		{
			return this->Memory.Memory;
		};

		inline size_t get_capacity()
		{
			return this->Memory.Capacity;
		};

		inline char empty()
		{
			return this->Size == 0;
		};

		inline ElementType* get(const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index);
#endif
			return &this->Memory.Memory[p_index];
		};

		inline ElementType get_rv(const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index);
#endif
			return this->Memory.Memory[p_index];
		};



		inline void clear()
		{
			this->Size = 0;
		};


		inline char insert_array_at(const Slice<ElementType>* p_elements, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_array_at_always instead.
#endif

			return this->insert_array_at_unchecked(p_elements, p_index);
		};

		inline char insert_array_at_1v(const Slice<ElementType> p_elements, const size_t p_index)
		{
			return this->insert_array_at(&p_elements, p_index);
		};



		inline char insert_element_at(const ElementType* p_element, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // cannot insert at head. Use vector_insert_element_at_always instead.
#endif

			return this->insert_element_at_unchecked(p_element, p_index);
		};

		inline char insert_element_at_1v(const ElementType p_element, const size_t p_index)
		{
			return this->insert_element_at(&p_element, p_index);
		};

		inline char push_back_array(const Slice<ElementType>* p_elements)
		{
			this->Memory.resize_until_capacity_met(this->Size + p_elements->Size);
			this->Memory.copy_memory(this->Size, p_elements);
			this->Size += p_elements->Size;

			return 1;
		};

		inline char push_back_array_1v(const Slice<ElementType> p_elements)
		{
			return this->push_back_array(&p_elements);
		};

		inline char push_back_element_empty()
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->Size += 1;
			return 1;
		};

		inline char push_back_element(const ElementType* p_element)
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->Memory.Memory[this->Size] = *p_element;
			this->Size += 1;

			return 1;
		};

		inline char push_back_element_1v(const ElementType p_element)
		{
			return this->push_back_element(&p_element);
		};



		inline char insert_array_at_always(const Slice<ElementType>* p_elements, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
#endif
			if (p_index == this->Size)
			{
				return this->push_back_array(p_elements);
			}
			else
			{
				return this->insert_array_at_unchecked(p_elements, p_index);
			}
		};


		inline char insert_element_at_always(const ElementType* p_element, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
#endif

			if (p_index == this->Size)
			{
				return this->push_back_element(p_element);
			}
			else
			{
				return this->insert_element_at_unchecked(p_element, p_index);
			}
		};


		inline char erase_array_at(const size_t p_index, const size_t p_element_nb)
		{

#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_check(p_index + p_element_nb);
			this->bound_head_check(p_index); // use vector_pop_back_array //TODO -> create a "always" variant of vector_erase_array_at
#endif

			this->move_memory_up(p_index, p_element_nb);
			this->Size -= p_element_nb;

			return 1;
		};

		inline char erase_element_at(const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->bound_check(p_index);
			this->bound_head_check(p_index); // use vector_pop_back //TODO -> create a "always" variant of vector_erase_element_at
#endif

			this->move_memory_up(p_index, 1);
			this->Size -= 1;

			return 1;
		};


		inline char pop_back_array(const size_t p_element_nb)
		{
			this->Size -= p_element_nb;
			return 1;
		};

		inline char pop_back()
		{
			this->Size -= 1;
			return 1;
		};


	private:

		inline void bound_check(const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			if (p_index > this->Size)
			{
				abort();
			}
#endif
		};

		inline void bound_head_check(const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			if (p_index == this->Size)
			{
				abort();
			}
#endif
		};


		inline void move_memory_down(const size_t p_break_index, const size_t p_move_delta)
		{
			this->Memory.move_memory_down(this->Size, p_break_index, p_move_delta);
		};

		inline void move_memory_up(const size_t p_break_index, const size_t p_move_delta)
		{
			this->Memory.move_memory_up(this->Size, p_break_index, p_move_delta);
		};

		inline char insert_element_at_unchecked(const ElementType* p_element, const size_t p_index)
		{
			this->Memory.resize_until_capacity_met(this->Size + 1);
			this->move_memory_down(p_index, 1);
			this->Memory.Memory[p_index] = *p_element;
			this->Size += 1;

			return 1;
		};

		inline char insert_array_at_unchecked(const Slice<ElementType>* p_elements, const size_t p_index)
		{
			this->Memory.resize_until_capacity_met(this->Size + p_elements->Size);
			this->move_memory_down(p_index, p_elements->Size);
			this->Memory.copy_memory(p_index, p_elements);

			this->Size += p_elements->Size;

			return 1;
		};

	};

}

#define vector_loop(VectorVariable, Iteratorname) size_t Iteratorname = 0; Iteratorname < (VectorVariable)->Size; Iteratorname++
#define vector_loop_reverse(VectorVariable, Iteratorname) size_t Iteratorname = (VectorVariable)->Size - 1; Iteratorname != -1; --Iteratorname

