#pragma once

//TODO -> delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	using PoolMemory_t = Vector<ElementType>;

	template<class ElementType>
	using PoolFreeBlocks_t = Vector<Token(ElementType)>;

	/*
		A Pool is a non continous Vector where elements are acessed via Tokens.
		Generated Tokens are unique from it's source Pool.
		Even if pool memory is reallocate, generated Tokens are still valid.
		/!\ It is very unsafe to store raw pointer of an element. Because is Pool memory is reallocated, then the pointer is no longer valid.
	*/
	template<class ElementType>
	struct Pool
	{
		PoolMemory_t<ElementType> memory;
		PoolFreeBlocks_t<ElementType> free_blocks;

		inline static Pool<ElementType> build(const PoolMemory_t<ElementType> p_memory, const PoolFreeBlocks_t<ElementType> p_free_blocks)
		{
			return Pool<ElementType>{p_memory, p_free_blocks};
		};

		inline static Pool<ElementType> allocate(const uimax p_memory_capacity)
		{
			return Pool<ElementType>{ Vector<ElementType>::allocate(p_memory_capacity), Vector<Token(ElementType)>::build(cast(Token(ElementType)*, NULL), 0) };
		};


		inline void free()
		{
			this->memory.free();
			this->free_blocks.free();
		};

		inline uimax get_size()
		{
			return this->memory.Size;
		};

		inline uimax get_capacity()
		{
			return this->memory.Memory.Capacity;
		};

		inline uimax get_free_size()
		{
			return this->free_blocks.Size;
		};

		inline ElementType* get_memory()
		{
			return this->memory.Memory.Memory;
		};

		inline int8 has_allocated_elements()
		{
			return this->memory.Size != this->free_blocks.Size;
		};

		inline int8 is_element_free(const Token(ElementType) p_token)
		{
			for (vector_loop(&this->free_blocks, i))
			{
				if (tk_v(this->free_blocks.get(i)) == tk_v(p_token))
				{
					return 1;
				};
			};

			return 0;
		};

		inline ElementType& get(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			this->element_free_check(p_token);
#endif

			return this->memory.get(tk_v(p_token));
		};

		inline Token(ElementType) alloc_element_empty()
		{
			if (!this->free_blocks.empty())
			{
				Token(ElementType) l_availble_token = this->free_blocks.get(this->free_blocks.Size - 1);
				this->free_blocks.pop_back();
				return l_availble_token;
			}
			else
			{
				this->memory.push_back_element_empty();
				return Token(ElementType) { this->memory.Size - 1 };
			}
		}

		inline Token(ElementType) alloc_element(const ElementType& p_element)
		{
			if (!this->free_blocks.empty())
			{
				Token(ElementType) l_availble_token = this->free_blocks.get(this->free_blocks.Size - 1);
				this->free_blocks.pop_back();
				this->memory.get(tk_v(l_availble_token)) = p_element;
				return l_availble_token;
			}
			else
			{
				this->memory.push_back_element(p_element);
				return Token(ElementType) { this->memory.Size - 1 };
			}
		};


		inline void release_element(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			this->element_not_free_check(p_token);
#endif

			this->free_blocks.push_back_element(p_token);
		};

		inline void release_element_1v(const Token(ElementType) p_token)
		{
			this->release_element(p_token);
		};

	private:

		inline void element_free_check(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};

		inline void element_not_free_check(const Token(ElementType) p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};
	};


}
