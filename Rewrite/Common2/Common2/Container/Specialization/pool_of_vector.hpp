#pragma once

//TODO delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	using PoolOfVectorMemory_t = VectorOfVector<ElementType>;

	template<class ElementType>
	using PoolOfVectorToken = Token<VectorOfVector_Element<ElementType>>;

	template<class ElementType>
	using PoolOfVectorFreeBlocks_t = Vector<PoolOfVectorToken<ElementType>>;

	/*
		A PoolOfVector is a wrapped VectorOfVector with pool allocation logic.
		Any operation on nested vectors must be called with the (poolofvector_element_*) functions
	*/
	template<class ElementType>
	struct PoolOfVector
	{
		PoolOfVectorMemory_t<ElementType> Memory;
		PoolOfVectorFreeBlocks_t<ElementType> FreeBlocks;


		inline static PoolOfVector<ElementType> allocate_default()
		{
			return PoolOfVector<ElementType>
			{
				VectorOfVector<ElementType>::allocate_default(),
					Vector<PoolOfVectorToken<ElementType>>::allocate(0)
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->FreeBlocks.free();
		};


		inline char is_token_free(const PoolOfVectorToken<ElementType>* p_token)
		{
			for (vector_loop(&this->FreeBlocks, i))
			{
				if (this->FreeBlocks.get(i)->tok == p_token->tok)
				{
					return 1;
				}
			}
			return 0;
		};


		inline PoolOfVectorToken<ElementType> alloc_vector_with_values(const Slice<ElementType>* p_initial_elements)
		{
			if (!this->FreeBlocks.empty())
			{
				PoolOfVectorToken<ElementType> l_token = this->FreeBlocks.get_rv(this->FreeBlocks.Size - 1);
				this->FreeBlocks.pop_back();
				this->Memory.element_push_back_array(l_token.tok, p_initial_elements);
				return l_token;
			}
			else
			{
				this->Memory.push_back_element(p_initial_elements);
				return PoolOfVectorToken<ElementType>{ this->Memory.varying_vector.get_size() - 1 };
			}
		};

		inline PoolOfVectorToken<ElementType> alloc_vector()
		{
			if (!this->FreeBlocks.empty())
			{
				PoolOfVectorToken<ElementType> l_token = this->FreeBlocks.get_rv(this->FreeBlocks.Size - 1);
				this->FreeBlocks.pop_back();
				return l_token;
			}
			else
			{
				this->Memory.push_back();
				return PoolOfVectorToken<ElementType>{ this->Memory.varying_vector.get_size() - 1 };
			}
		};

		inline void release_vector(const PoolOfVectorToken<ElementType>* p_token)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			this->Memory.element_clear(p_token->tok);
			this->FreeBlocks.push_back_element(p_token);
		};

		inline VectorOfVector_Element<ElementType> get_vector(const PoolOfVectorToken<ElementType>* p_token)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			return this->Memory.get(p_token->tok);
		};

		inline VectorOfVector_Element<ElementType> get_vector_1v(const PoolOfVectorToken<ElementType> p_token)
		{
			return this->get_vector(&p_token);
		};

		inline void element_push_back_element(const PoolOfVectorToken<ElementType>* p_token, const ElementType* p_element)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif

			this->Memory.element_push_back_element(p_token->tok, p_element);
		};

		inline void element_push_back_element_2v(const PoolOfVectorToken<ElementType>* p_token, const ElementType p_element)
		{
			this->element_push_back_element(p_token, &p_element);
		};

		inline void element_erase_element_at(const PoolOfVectorToken<ElementType>* p_token, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif
			this->Memory.element_erase_element_at(p_token->tok, p_index);
		};

		inline void element_erase_element_at_always(const PoolOfVectorToken<ElementType>* p_token, const size_t p_index)
		{
#if CONTAINER_BOUND_TEST
			this->token_not_free_check(p_token);
#endif
			this->Memory.element_erase_element_at_always(p_token->tok, p_index);
		};

	private:
		inline void token_not_free_check(const PoolOfVectorToken<ElementType>* p_token)
		{
#if CONTAINER_BOUND_TEST
			if (this->is_token_free(p_token))
			{
				abort();
			}
#endif
		};

	};

}

