#pragma once

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

	inline static Pool<ElementType> allocate(const size_t p_memory_capacity)
	{
		return Pool<ElementType>{ Vector<ElementType>::allocate(p_memory_capacity), Vector<Token(ElementType)>::build(cast(Token(ElementType)*, NULL), 0) };
	};


	inline void free()
	{
		this->memory.free();
		this->free_blocks.free();
	};

	inline size_t get_size()
	{
		return this->memory.Size;
	};

	inline size_t get_capacity()
	{
		return this->memory.Span.Capacity;
	};

	inline size_t get_free_size()
	{
		return this->free_blocks.Size;
	};

	inline ElementType* get_memory()
	{
		return this->memory.Span.Memory;
	};

	inline char is_element_free(const Token<ElementType>* p_token)
	{
		for (vector_loop(&this->free_blocks, i))
		{
			if (this->free_blocks.get(i)->tok == p_token->tok)
			{
				return 1;
			};
		};

		return 0;
	};

	inline char is_element_free_1v(const Token<ElementType> p_token)
	{
		return this->is_element_free(&p_token);
	};


	inline ElementType* get(const Token<ElementType>* p_token)
	{
#if CONTAINER_BOUND_TEST
		this->element_free_check(p_token);
#endif

		return this->memory.get(p_token->tok);
	};

	inline ElementType* get_1v(const Token<ElementType> p_token)
	{
		return this->get(&p_token);
	};

	inline ElementType get_rv1v(const Token<ElementType> p_token)
	{
		return *this->get(&p_token);
	};

	inline Token<ElementType> alloc_element_empty()
	{
		if (!this->free_blocks.empty())
		{
			Token(ElementType) l_availble_token = this->free_blocks.get_rv(this->free_blocks.Size - 1);
			this->free_blocks.pop_back();
			return l_availble_token;
		}
		else
		{
			this->memory.push_back_element_empty();
			return Token(ElementType) { this->memory.Size - 1 };
		}
	}

	inline Token<ElementType> alloc_element(const ElementType* p_element)
	{
		if (!this->free_blocks.empty())
		{
			Token(ElementType) l_availble_token = this->free_blocks.get_rv(this->free_blocks.Size - 1);
			this->free_blocks.pop_back();
			*this->memory.get(l_availble_token.tok) = *p_element;
			return l_availble_token;
		}
		else
		{
			this->memory.push_back_element(p_element);
			return Token(ElementType) { this->memory.Size - 1 };
		}
	};


	inline Token<ElementType> alloc_element_1v(const ElementType p_element)
	{
		return this->alloc_element(&p_element);
	}

	inline void release_element(const Token<ElementType>* p_token)
	{
#if CONTAINER_BOUND_TEST
		this->element_not_free_check(p_token);
#endif

		this->free_blocks.push_back_element(p_token);
	};

	inline void release_element_1v(const Token<ElementType> p_token)
	{
		this->release_element(&p_token);
	};

	private:

		inline void element_free_check(const Token<ElementType>* p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};

		inline void element_free_check_1v(const Token<ElementType> p_token)
		{
			this->element_free_check(&p_token);
		};

		inline void element_not_free_check(const Token<ElementType>* p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_element_free(p_token))
			{
				abort();
			}
#endif
		};

		inline void element_not_free_check_1v(Pool<ElementType>* p_pool, const Token<ElementType> p_token)
		{
			this->element_not_free_check(&p_token);
		};

};



#define pool_loop(PoolVariable, Iteratorname) size_t Iteratorname = 0; Iteratorname < (PoolVariable)->get_size(); Iteratorname++