#pragma once

template<class ElementType>
struct PoolIndexed
{
	Pool<ElementType> Pool;
	Vector<Token(ElementType)> Indices;

	inline static PoolIndexed<ElementType> allocate_default()
	{
		return PoolIndexed<ElementType>
		{
			::Pool<ElementType>::allocate(0),
			Vector<Token(ElementType)>::allocate(0)
		};
	};

	inline void free()
	{
		this->Pool.free();
		this->Indices.free();
	};

	inline Token(ElementType) alloc_element(const ElementType* p_element)
	{
		Token(ElementType) l_token = this->Pool.alloc_element(p_element);
		this->Indices.push_back_element(&l_token);
		return l_token;
	};

	inline void release_element(const Token(ElementType)* p_element)
	{
		this->Pool.release_element(p_element);
		for (vector_loop(&this->Indices, i))
		{
			if (this->Indices.get(i)->tok == p_element->tok)
			{
				this->Indices.erase_element_at(i);
				break;
			}
		};
	};

	inline ElementType* get(const Token(ElementType)* p_element)
	{
		return this->Pool.get(p_element);
	};

};



