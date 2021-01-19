#pragma once

//TODO delete when common2 migration is complete
namespace v2
{
	template<class ElementType>
	struct PoolIndexed
	{
		Pool<ElementType> Memory;
		Vector<Token(ElementType)> Indices;

		inline static PoolIndexed<ElementType> allocate_default()
		{
			return PoolIndexed<ElementType>
			{
				Pool<ElementType>::allocate(0),
					Vector<Token(ElementType)>::allocate(0)
			};
		};

		inline void free()
		{
			this->Memory.free();
			this->Indices.free();
		};

		inline int8 has_allocated_elements()
		{
			return this->Memory.has_allocated_elements();
		};

		inline Token(ElementType) alloc_element(const ElementType& p_element)
		{
			Token(ElementType) l_token = this->Memory.alloc_element(p_element);
			this->Indices.push_back_element(l_token);
			return l_token;
		};

		inline void release_element(const Token(ElementType) p_element)
		{
			this->Memory.release_element(p_element);
			for (vector_loop(&this->Indices, i))
			{
				if (tk_eq(this->Indices.get(i), p_element))
				{
					this->Indices.erase_element_at(i);
					break;
				}
			};
		};

		inline ElementType& get(const Token(ElementType) p_element)
		{
			return this->Memory.get(p_element);
		};

	};




}
