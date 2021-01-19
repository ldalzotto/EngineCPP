#pragma once

//TODO -> delete when common2 migration is complete
namespace v2
{
	/*
		A VaryingVector is a Vector which elements can have different sizes.
		Elements are accessed vie the Chunks_t lookup table.
		Memory is continuous.
	*/
	struct VaryingVector
	{
		using Memory_t = Vector<int8>;
		using Chunks_t = Vector<SliceIndex>;

		Memory_t memory;
		Chunks_t chunks;

		inline static  VaryingVector build(const Memory_t& p_memory, const Chunks_t& p_chunks)
		{
			return VaryingVector{ p_memory, p_chunks };
		};

		inline static VaryingVector allocate(const uimax p_memory_array_initial_capacity, const uimax p_chunk_array_initial_capacity)
		{
			return build(Memory_t::allocate(p_memory_array_initial_capacity), Chunks_t::allocate(p_chunk_array_initial_capacity));
		};

		inline static VaryingVector allocate_default()
		{
			return allocate(0, 0);
		};


		inline void free()
		{
			this->memory.free();
			this->chunks.free();
		};

		inline uimax get_size()
		{
			return this->chunks.Size;
		};

		inline void push_back(const Slice<int8>& p_bytes)
		{
			SliceIndex l_chunk = SliceIndex::build(this->memory.Size, p_bytes.Size);
			this->memory.push_back_array(p_bytes);
			this->chunks.push_back_element(l_chunk);
		};

		template<class ElementType>
		inline void push_back_element(const ElementType& p_element)
		{
			this->push_back(Slice<int8>::build_memory_elementnb(cast(int8*, &p_element), sizeof(ElementType)));
		};

		inline void pop_back()
		{
			this->memory.pop_back_array(
				this->chunks.get(this->chunks.Size - 1).Size
			);
			this->chunks.pop_back();
		};

		inline void insert_at(const Slice<int8>& p_bytes, const uimax p_index)
		{
			SliceIndex& l_break_chunk = this->chunks.get(p_index);
			this->memory.insert_array_at(p_bytes, l_break_chunk.Begin);
			this->chunks.insert_element_at(SliceIndex::build(l_break_chunk.Begin, p_bytes.Size), p_index);

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += p_bytes.Size;
			}
		};

		inline void erase_element_at(const uimax p_index)
		{
			SliceIndex& l_chunk = this->chunks.get(p_index);
			this->memory.erase_array_at(
				l_chunk.Begin,
				l_chunk.Size
			);

			for (loop(i, p_index, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= l_chunk.Size;
			};

			this->chunks.erase_element_at(p_index);
		};

		inline void erase_array_at(const uimax p_index, const uimax p_element_nb)
		{
#if CONTAINER_BOUND_TEST
			assert_true(p_element_nb != 0);
#endif	

			SliceIndex l_removed_chunk = SliceIndex::build_default();

			for (loop(i, p_index, p_index + p_element_nb))
			{
				l_removed_chunk.Size += this->chunks.get(i).Size;
			};

			SliceIndex& l_first_chunk = this->chunks.get(p_index);
			l_removed_chunk.Begin = l_first_chunk.Begin;

			this->memory.erase_array_at(
				l_removed_chunk.Begin,
				l_removed_chunk.Size
			);

			for (loop(i, p_index + p_element_nb, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= l_removed_chunk.Size;
			};

			this->chunks.erase_array_at(p_index, p_element_nb);
		};



		inline void element_expand(const uimax p_index, const uimax p_expansion_size)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_expansion_size != 0);
#endif

			uimax l_new_varyingvector_size = this->memory.Size + p_expansion_size;

			this->memory.Memory.resize_until_capacity_met(l_new_varyingvector_size);
			l_updated_chunk.Size += p_expansion_size;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += p_expansion_size;
			}
		};

		inline void element_expand_with_value(const uimax p_index, const Slice<int8>& p_pushed_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_pushed_element.Size != 0);
#endif

			uimax l_size_delta = p_pushed_element.Size;
			uimax l_new_varyingvector_size = this->memory.Size + l_size_delta;

			this->memory.Memory.resize_until_capacity_met(l_new_varyingvector_size);

			this->memory.insert_array_at_always(p_pushed_element, l_updated_chunk.Begin + l_updated_chunk.Size);
			l_updated_chunk.Size += l_size_delta;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin += l_size_delta;
			}
		};

		inline void element_shrink(const uimax p_index, const uimax p_size_delta)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);

#if CONTAINER_BOUND_TEST
			assert_true(p_size_delta != 0);
			assert_true(p_size_delta <= l_updated_chunk.Size);
#endif

			this->memory.erase_array_at(l_updated_chunk.Begin + l_updated_chunk.Size - p_size_delta, p_size_delta);
			l_updated_chunk.Size -= p_size_delta;

			for (loop(i, p_index + 1, this->chunks.Size))
			{
				this->chunks.get(i).Begin -= p_size_delta;
			}
		};

		inline void element_writeto(const uimax p_index, const uimax p_insertion_offset, const Slice<int8>& p_inserted_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);
			Slice<int8> l_updated_chunk_slice = Slice<int8>::build_asint8_memory_elementnb(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size).slide_rv(p_insertion_offset);

			slice_memcpy(l_updated_chunk_slice, p_inserted_element);
		};

		inline void element_movememory(const uimax p_index, const uimax p_insertion_offset, const Slice<int8>& p_inserted_element)
		{
			SliceIndex& l_updated_chunk = this->chunks.get(p_index);
			Slice<int8> l_updated_chunk_slice = Slice<int8>::build_asint8_memory_elementnb(this->memory.get_memory() + l_updated_chunk.Begin, l_updated_chunk.Size).slide_rv(p_insertion_offset);

			slice_memmove(l_updated_chunk_slice, p_inserted_element);
		};


		inline Slice<int8> get(const uimax p_index)
		{
			SliceIndex& l_chunk = this->chunks.get(p_index);
			return Slice<int8>::build_memory_offset_elementnb(
				this->memory.get_memory(),
				l_chunk.Begin,
				l_chunk.Size
			);
		};

		template<class ElementType>
		inline Slice<ElementType> get_element(const uimax p_index)
		{
			return slice_cast<ElementType>(
				this->get(p_index)
				);
		};

	};
}

