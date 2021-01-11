#pragma once

namespace v2
{
	struct Heap
	{
		enum class AllocationState : short int
		{
			NOT_ALLOCATED = 0,
			ALLOCATED = 1,
			HEAP_RESIZED = 2
		};

		Pool<SliceIndex> AllocatedChunks;
		Vector<SliceIndex> FreeChunks;
		size_t Size;

		inline static Heap allocate(const size_t p_heap_size)
		{
			Heap l_heap = Heap{
				Pool<SliceIndex>::allocate(0),
				Vector<SliceIndex>::allocate(1),
				p_heap_size
			};
			l_heap.FreeChunks.push_back_element_1v(SliceIndex::build(0, p_heap_size));
			return l_heap;
		};

		inline void free()
		{
			this->AllocatedChunks.free();
			this->FreeChunks.free();
			this->Size = 0;
		};

		inline void resize(const size_t p_newsize)
		{
			size_t l_old_size = this->Size;
			this->FreeChunks.push_back_element_1v(SliceIndex::build(l_old_size, p_newsize - l_old_size));
		};

		inline AllocationState allocate_element(const size_t p_size, Token(SliceIndex)* out_chunk)
		{
			if (!_allocate_element(p_size, out_chunk))
			{
				this->defragment();
				if (!_allocate_element(p_size, out_chunk))
				{
					this->resize(this->Size == 0 ? p_size : ((this->Size * 2) + p_size));

#if CONTAINER_MEMORY_TEST
					assert_true(
#endif
						_allocate_element(p_size, out_chunk)
#if CONTAINER_MEMORY_TEST
					)
#endif
					;

					return cast(AllocationState, cast(short int, AllocationState::ALLOCATED) & cast(short int, AllocationState::HEAP_RESIZED));
				}
				return AllocationState::ALLOCATED;
			}
			return AllocationState::ALLOCATED;
		};

		inline AllocationState allocate_element_with_alignment(const size_t p_size, const size_t p_alignement_modulo, Token(SliceIndex)* out_chunk)
		{
			if (!_allocate_element_with_alignment(p_size, p_alignement_modulo, out_chunk))
			{
				this->defragment();
				if (!_allocate_element_with_alignment(p_size, p_alignement_modulo, out_chunk))
				{
					this->resize(this->Size == 0 ? (p_alignement_modulo > p_size ? p_alignement_modulo : p_size) : ((this->Size * 2) + (p_alignement_modulo > p_size ? p_alignement_modulo : p_size)));

#if CONTAINER_MEMORY_TEST
					assert_true(
#endif
						_allocate_element_with_alignment(p_size, p_alignement_modulo, out_chunk)
#if CONTAINER_MEMORY_TEST
						)
#endif
					;

					return cast(AllocationState, cast(short int, AllocationState::ALLOCATED) & cast(short int, AllocationState::HEAP_RESIZED));
				}
				return AllocationState::ALLOCATED;
			}
			return AllocationState::NOT_ALLOCATED;
		};

		inline SliceIndex* get(const Token(SliceIndex)* p_chunk)
		{
			return this->AllocatedChunks.get(p_chunk);
		};

		inline void release_element(const Token(SliceIndex)* p_chunk)
		{
			this->FreeChunks.push_back_element(this->AllocatedChunks.get(p_chunk));
			this->AllocatedChunks.release_element(p_chunk);
		};

		inline AllocationState reallocate_element(const Token(SliceIndex)* p_chunk, const size_t p_new_size, Token(SliceIndex)* out_chunk)
		{
			AllocationState l_allocation = this->allocate_element(p_new_size, out_chunk);
			if (cast(short int, l_allocation) & cast(short int, AllocationState::ALLOCATED))
			{
				this->release_element(p_chunk);
			}
			return l_allocation;
		};

		inline void defragment()
		{
			//TODO
		};

	private:

		inline char _allocate_element(const size_t p_size, Token(SliceIndex)* out_chunk)
		{
#if CONTAINER_BOUND_TEST
			assert_true(p_size != 0);
#endif

			for (size_t i = 0; i < this->FreeChunks.Size; i++)
			{
				SliceIndex* l_free_chunk = this->FreeChunks.get(i);
				if (l_free_chunk->Size > p_size)
				{
					SliceIndex l_new_allocated_chunk;
					l_free_chunk->slice_two(l_free_chunk->Begin + p_size, &l_new_allocated_chunk, l_free_chunk);
					*out_chunk = this->AllocatedChunks.alloc_element(&l_new_allocated_chunk);
					return true;
				}
				else if (l_free_chunk->Size == p_size)
				{
					*out_chunk = this->AllocatedChunks.alloc_element(l_free_chunk);
					this->FreeChunks.erase_element_at(i);
					return true;
				}
			}

			return false;
		};

		inline char _allocate_element_with_alignment(const size_t p_size, const size_t p_alignement_modulo, Token(SliceIndex)* out_chunk)
		{
#if CONTAINER_BOUND_TEST
			assert_true(p_size != 0);
#endif
			for (size_t i = 0; i < this->FreeChunks.Size; i++)
			{
				SliceIndex* l_free_chunk = this->FreeChunks.get(i);

				if (l_free_chunk->Size > p_size)
				{
					size_t l_offset_modulo = (l_free_chunk->Begin % p_alignement_modulo);
					if (l_offset_modulo == 0)
					{
						// create one free chunk (after)
						SliceIndex l_new_allocated_chunk;
						l_free_chunk->slice_two(l_free_chunk->Begin + p_size, &l_new_allocated_chunk, l_free_chunk);
						*out_chunk = this->AllocatedChunks.alloc_element(&l_new_allocated_chunk);
						return true;
					}
					else
					{
						size_t l_chunk_offset_delta = p_alignement_modulo - l_offset_modulo;
						// Does the offsetted new memory is able to be allocated in the chunk ?
						if (l_free_chunk->Size > (p_size + l_chunk_offset_delta)) //offsetted chunk is in the middle of the free chunk
						{
							//create two free chunk (before and after)

							SliceIndex l_new_allocated_chunk, l_new_free_chunk, l_tmp_chunk;
							l_free_chunk->slice_two(l_free_chunk->Begin + l_chunk_offset_delta, l_free_chunk, &l_tmp_chunk);
							l_free_chunk->slice_two(l_free_chunk->Begin + p_size, &l_new_allocated_chunk, &l_new_free_chunk);

							*out_chunk = this->AllocatedChunks.alloc_element(&l_new_allocated_chunk);
							this->FreeChunks.push_back_element(&l_new_free_chunk);

							return true;
						}
						else if (l_free_chunk->Size == (p_size + l_chunk_offset_delta)) //offsetted chunk end matches perfectly the end of the free chunk
						{
							SliceIndex l_new_allocated_chunk;
							l_free_chunk->slice_two(l_free_chunk->Begin + l_chunk_offset_delta, l_free_chunk, &l_new_allocated_chunk);
							*out_chunk = this->AllocatedChunks.alloc_element(&l_new_allocated_chunk);

							return true;
						}
					}

				}
				else if (l_free_chunk->Size == p_size)
				{
					size_t l_offset_modulo = (l_free_chunk->Size % p_alignement_modulo);
					if (l_offset_modulo == 0)
					{
						*out_chunk = this->AllocatedChunks.alloc_element(l_free_chunk);
						this->FreeChunks.erase_element_at(i);

						return true;
					}
				}
			}

			return false;
		};

	};
}

