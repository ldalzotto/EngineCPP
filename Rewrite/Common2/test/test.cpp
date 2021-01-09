
#include "Common2/common2.hpp"

#include <stdio.h>

namespace v2
{

	template<class ElementType>
	inline void assert_span_unitialized(Span<ElementType>* p_span)
	{
		assert_true(p_span->Capacity == 0);
		assert_true(p_span->Memory == NULL);
	};

	inline void span_test()
	{
		Span<size_t> l_span_sizet = Span<size_t>::build(NULL, 0);

		//When resizing the span, new memory is allocated
		{
			size_t l_new_capacity = 10;
			l_span_sizet.resize(10);
			assert_true(l_span_sizet.Capacity == l_new_capacity);
			assert_true(l_span_sizet.Memory != NULL);
		}

		//When freeing the span, it's structure is resetted
		{
			l_span_sizet.free();
			assert_span_unitialized(&l_span_sizet);
		}
	};

	inline void vector_test()
	{
		Vector<size_t> l_vector_sizet = Vector<size_t>::build((size_t*)NULL, 0);

		// vector_push_back_array
		{
			size_t l_old_size = l_vector_sizet.Size;
			size_t l_elements[5] = { 0,1,2,3,4 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 5);

			l_vector_sizet.push_back_array(&l_elements_slice);
			assert_true(l_vector_sizet.Size == l_old_size + 5);
			for (loop(i, l_old_size, l_vector_sizet.Size))
			{
				assert_true(*l_vector_sizet.get(i) == l_elements[i - l_old_size]);
			}
		}

		// vector_push_back_element
		{
			size_t l_old_size = l_vector_sizet.Size;
			size_t l_element = 25;
			l_vector_sizet.push_back_element(&l_element);
			assert_true(l_vector_sizet.Size == l_old_size + 1);
			assert_true(*l_vector_sizet.get(l_vector_sizet.Size - 1) == l_element);
		}

		// vector_insert_array_at
		{
			size_t l_old_size = l_vector_sizet.Size;
			size_t l_elements[5] = { 0,1,2,3,4 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 5);
			l_vector_sizet.insert_array_at(&l_elements_slice, 0);
			assert_true(l_vector_sizet.Size == l_old_size + 5);
			for (loop_si(i, 0, 5))
			{
				assert_true((*l_vector_sizet.get(i)) == i);
			}

			l_vector_sizet.insert_array_at(&l_elements_slice, 3);
			assert_true(l_vector_sizet.Size == l_old_size + 10);
			for (loop_si(i, 0, 3))
			{
				assert_true((*l_vector_sizet.get(i)) == l_elements[i]);
			}
			//Middle insertion
			for (loop_si(i, 3, 8))
			{
				assert_true((*l_vector_sizet.get(i)) == l_elements[i - cast(size_t, 3)]);
			}
			for (loop_si(i, 8, 10))
			{
				assert_true((*l_vector_sizet.get(i)) == l_elements[i - cast(size_t, 5)]);
			}
		}

		// vector_insert_element_at
		{
			size_t l_element = 20;
			size_t l_old_size = l_vector_sizet.Size;

			l_vector_sizet.insert_element_at(&l_element, 7);
			assert_true(*l_vector_sizet.get(7) == l_element);
			assert_true(l_vector_sizet.Size == l_old_size + 1);

			l_vector_sizet.insert_element_at_1v(cast(size_t, 20), 9);
		}

		// vector_erase_element_at
		{
			size_t l_old_size = l_vector_sizet.Size;
			size_t l_erase_index = 1;
			size_t l_element_after = *l_vector_sizet.get(l_erase_index + 1);
			l_vector_sizet.erase_element_at(1);
			assert_true(l_vector_sizet.Size == l_old_size - 1);
			assert_true(*l_vector_sizet.get(1) == l_element_after);
		}

		// vector_erase_array_at
		{
			size_t l_old_size = l_vector_sizet.Size;
			size_t l_erase_begin_index = 3;
			const size_t l_erase_nb = 6;
			const size_t l_old_element_check_nb = 3;

			size_t l_old_values[l_old_element_check_nb];
			for (loop(i, l_erase_begin_index + l_erase_nb, (l_erase_begin_index + l_erase_nb) + l_old_element_check_nb))
			{
				l_old_values[i - (l_erase_begin_index + l_erase_nb)] = *l_vector_sizet.get(i);
			}

			l_vector_sizet.erase_array_at(l_erase_begin_index, l_erase_nb);

			assert_true(l_vector_sizet.Size == l_old_size - l_erase_nb);
			for (loop(i, 0, l_old_element_check_nb))
			{
				assert_true(*l_vector_sizet.get(l_erase_begin_index + i) == l_old_values[i]);
			}
		}

		// vector_pop_back
		{
			size_t l_old_size = l_vector_sizet.Size;
			l_vector_sizet.pop_back();
			assert_true(l_vector_sizet.Size == l_old_size - 1);
		}

		// vector_pop_back_array
		{
			size_t l_old_size = l_vector_sizet.Size;
			l_vector_sizet.pop_back_array(3);
			assert_true(l_vector_sizet.Size == l_old_size - 3);
		}

		//When freeing the vcetor, it's structure is resetted
		{
			l_vector_sizet.free();
			assert_true(l_vector_sizet.Size == 0);
			assert_span_unitialized(&l_vector_sizet.Memory);
		}
	};

	inline void pool_test()
	{
		Pool<size_t> l_pool_sizet = Pool<size_t>::allocate(10);

		{
			assert_true(l_pool_sizet.get_memory() != NULL);
			assert_true(l_pool_sizet.get_capacity() == 10);
			assert_true(l_pool_sizet.get_size() == 0);
		}

		// pool_alloc_element - allocate new element
		{
			assert_true(l_pool_sizet.get_free_size() == 0);

			size_t l_element = 3;
			Token(size_t) l_token = l_pool_sizet.alloc_element(&l_element);

			assert_true(l_token.tok == 0);
			assert_true(*l_pool_sizet.get(&l_token) == l_element);
		}

		// pool_release_element - release elements
		{
			Token(size_t) l_token = Token(size_t) { 0 };
			l_pool_sizet.release_element(&l_token);

			// memory is not deallocated
			assert_true(l_pool_sizet.get_size() == 1);
		}

		// pool_alloc_element - allocating an element while there is free slots
		{
			size_t l_element = 4;
			Token(size_t) l_token = l_pool_sizet.alloc_element(&l_element);

			l_pool_sizet.alloc_element_1v(cast(size_t, 10));
			l_pool_sizet.release_element_1v(
				l_pool_sizet.alloc_element_1v(cast(size_t, 10))
			);
			l_pool_sizet.alloc_element_1v(cast(size_t, 10));

			assert_true(l_token.tok == 0);
			assert_true(*l_pool_sizet.get(&l_token) == l_element);
		}

		for (pool_loop(&l_pool_sizet, i))
		{
			size_t l_element = l_pool_sizet.get_rv1v(Token(size_t) { i });
		}

		{
			l_pool_sizet.free();
			assert_true(l_pool_sizet.get_size() == 0);
			assert_true(l_pool_sizet.get_capacity() == 0);
			assert_true(l_pool_sizet.get_memory() == 0);
		}
	};

	inline void varyingvector_test()
	{
		VaryingVector l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_push_back
		{
			assert_true(l_varyingvector.memory.Size == 0);

			Slice<char> l_slice = Slice<char>::build_memory_elementnb(cast(char*, &l_varyingvector), 10);
			l_varyingvector.push_back(&l_slice);

			assert_true(l_varyingvector.get_size() == 1);
			Slice<char> l_element_0 = l_varyingvector.get(0);
			assert_true(l_element_0.Size == 10);
			assert_true(slice_memcompare_element(&l_slice, &l_element_0));
		}

		// varyingvector_push_back_element
		{
			size_t l_element = 20;
			l_varyingvector.push_back_element(&l_element);

			size_t l_inserted_index = l_varyingvector.get_size() - 1;
			Slice<char> l_element_inserted = l_varyingvector.get(l_inserted_index);

			assert_true(l_element_inserted.Size == sizeof(size_t));
			assert_true(memory_compare(cast(const char*, &l_element), l_element_inserted.Begin, l_element_inserted.Size));

			Slice<size_t> l_casted_slice = slice_cast<size_t>(&l_element_inserted);
			assert_true(l_casted_slice.Size == 1);

		}

		// varyingvector_pop_back
		{

			size_t l_old_size = l_varyingvector.get_size();
			l_varyingvector.pop_back();
			assert_true(l_varyingvector.get_size() == (l_old_size - 1));
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_erase_element_at
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element_1v(cast(size_t, i));
			}

			assert_true(l_varyingvector.get_size() == 5);
			l_varyingvector.erase_element_at(2);
			assert_true(l_varyingvector.get_size() == 4);

			assert_true(*l_varyingvector.get_element<size_t>(2).Begin == 3);
			assert_true(*l_varyingvector.get_element<size_t>(3).Begin == 4);
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_erase_array_at
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element_1v(cast(size_t, i));
			}

			assert_true(l_varyingvector.get_size() == 5);
			l_varyingvector.erase_array_at(2, 2);
			assert_true(l_varyingvector.get_size() == 3);

			assert_true(*l_varyingvector.get_element<size_t>(2).Begin == 4);
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_element_expand_with_value varyingvector_element_shrink
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element_1v(cast(size_t, i));
			}

			size_t l_inserset_number = 30;
			Slice<char> l_expansion_slice = Slice<size_t>::build_aschar_memory_elementnb(&l_inserset_number, 1);
			l_varyingvector.element_expand_with_value(2, &l_expansion_slice);

			Slice<size_t> l_sizet_element_2 = slice_cast_0v<size_t>(l_varyingvector.get(2));
			assert_true(l_sizet_element_2.Size == 2);
			assert_true(l_sizet_element_2.get_rv(1) == l_inserset_number);

			{
				size_t* l_sizet_element_3 = slice_cast_singleelement_0v<size_t>(l_varyingvector.get(3));
				assert_true(*l_sizet_element_3 == 3);
			}

			l_varyingvector.element_shrink(2, sizeof(size_t));
			l_sizet_element_2 = slice_cast_0v<size_t>(l_varyingvector.get(2));
			assert_true(l_sizet_element_2.Size == 1);
			assert_true(l_sizet_element_2.get_rv(0) == 2);

			{
				size_t* l_sizet_element_3 = slice_cast_singleelement_0v<size_t>(l_varyingvector.get(3));
				assert_true(*l_sizet_element_3 == 3);
			}
		}

		// varyingvector_element_writeto
		{
			size_t l_element_0 = 10;
			size_t l_element_1 = 20;
			size_t l_element_2 = 30;

			l_varyingvector.element_expand(2, sizeof(size_t) * 3);
			l_varyingvector.element_writeto_3v(2, 0, Slice<size_t>::build_aschar_memory_singleelement(&l_element_0));
			l_varyingvector.element_writeto_3v(2, 2 * sizeof(size_t), Slice<size_t>::build_aschar_memory_singleelement(&l_element_2));
			l_varyingvector.element_writeto_3v(2, 1 * sizeof(size_t), Slice<size_t>::build_aschar_memory_singleelement(&l_element_1));


			Slice<char> l_varyingvector_element_2 = l_varyingvector.get(2);
			assert_true(*cast(size_t*, l_varyingvector_element_2.Begin) == l_element_0);
			assert_true(*l_varyingvector_element_2.slide_rv(sizeof(size_t)).Begin == l_element_1);
			assert_true(*l_varyingvector_element_2.slide_rv(2 * sizeof(size_t)).Begin == l_element_2);
		}

		{
			for (varyingvector_loop(&l_varyingvector, i))
			{
				l_varyingvector.get(i);
			}
		}

		{
			l_varyingvector.free();
			assert_true(l_varyingvector.get_size() == 0);
		}
	};

	inline void vectorofvector_test()
	{
		VectorOfVector<size_t> l_vectorofvector_size_t = VectorOfVector<size_t>::allocate_default();

		// vectorofvector_push_back vectorofvector_push_back_element
		{
			Span<size_t> l_sizets = Span<size_t>::allocate(10);
			for (loop(i, 0, l_sizets.Capacity))
			{
				*l_sizets.slice.get(i) = i;
			}

			l_vectorofvector_size_t.push_back();

			l_vectorofvector_size_t.push_back_element(&l_sizets.slice);
			VectorOfVector_Element<size_t> l_element =
				l_vectorofvector_size_t.get(
					l_vectorofvector_size_t.varying_vector.get_size() - 1
				);

			l_vectorofvector_size_t.push_back();

			assert_true(l_element.Header.Size == l_sizets.Capacity);
			for (loop(i, 0, l_sizets.Capacity))
			{
				assert_true(l_element.Memory.get_rv(i) == i);
			}

			l_sizets.free();
		}

		// vectorofvector_element_push_back_element
		{
			size_t l_index;
			for (loop(i, 0, 2))
			{
				l_vectorofvector_size_t.push_back();

				size_t l_element = 30;
				l_index = l_vectorofvector_size_t.varying_vector.get_size() - 2;
				l_vectorofvector_size_t.element_push_back_element(l_index, &l_element);
				VectorOfVector_Element<size_t> l_element_nested = l_vectorofvector_size_t.get(l_index);
				assert_true(l_element_nested.Header.Size == 1);
				assert_true(l_element_nested.Memory.get_rv(0) == l_element);

				l_element = 35;
				l_vectorofvector_size_t.element_clear(l_index);
				l_vectorofvector_size_t.element_push_back_element(l_index, &l_element);
				assert_true(l_element_nested.Header.Size == 1);
				assert_true(l_element_nested.Memory.get_rv(0) == l_element);
			}
		}

		// vectorofvector_element_insert_element_at
		{
			size_t l_elements[3] = { 100,120,140 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 3);
			l_vectorofvector_size_t.push_back_element(&l_elements_slice);
			size_t l_index = l_vectorofvector_size_t.varying_vector.get_size() - 1;

			size_t l_inserted_element = 200;
			l_vectorofvector_size_t.element_insert_element_at(l_index, 1, &l_inserted_element);

			VectorOfVector_Element<size_t> l_vector = l_vectorofvector_size_t.get(l_index);
			assert_true(l_vector.Header.Size == 4);
			assert_true(l_vector.Memory.get_rv(0) == l_elements[0]);
			assert_true(l_vector.Memory.get_rv(1) == l_inserted_element);
			assert_true(l_vector.Memory.get_rv(2) == l_elements[1]);
			assert_true(l_vector.Memory.get_rv(3) == l_elements[2]);
		}

		// vectorofvector_element_erase_element_at
		{
			size_t l_elements[3] = { 100,120,140 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 3);
			l_vectorofvector_size_t.push_back_element(&l_elements_slice);

			// size_t l_inserted_element = 200;
			size_t l_index = l_vectorofvector_size_t.varying_vector.get_size() - 1;
			l_vectorofvector_size_t.element_erase_element_at(l_index, 1);
			VectorOfVector_Element<size_t> l_vector = l_vectorofvector_size_t.get(l_index);
			assert_true(l_vector.Header.Size == 2);
			assert_true(l_vector.Memory.get_rv(0) == l_elements[0]);
			assert_true(l_vector.Memory.get_rv(1) == l_elements[2]);
		}

		// vectorofvector_element_push_back_array
		{
			size_t l_initial_elements[3] = { 1,2,3 };
			{
				Slice<size_t> l_initial_elements_slice = Slice<size_t>::build_memory_elementnb(l_initial_elements, 3);
				l_vectorofvector_size_t.push_back_element(&l_initial_elements_slice);
			}

			size_t l_index = l_vectorofvector_size_t.varying_vector.get_size() - 1;

			size_t l_elements[3] = { 100,120,140 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 3);

			size_t l_old_size = 0;
			{
				VectorOfVector_Element<size_t> l_vector_element = l_vectorofvector_size_t.get(l_index);
				l_old_size = l_vector_element.Header.Size;
			}

			l_vectorofvector_size_t.element_push_back_array(l_index, &l_elements_slice);

			{
				VectorOfVector_Element<size_t> l_vector_element = l_vectorofvector_size_t.get(l_index);
				assert_true(l_vector_element.Header.Size == l_old_size + 3);
				for (loop(i, 0, 3))
				{
					assert_true(l_vector_element.Memory.get_rv(i) == l_initial_elements[i]);
				}
				for (loop(i, l_old_size, l_old_size + 3))
				{
					assert_true(l_vector_element.Memory.get_rv(i) == l_elements[i - l_old_size]);
				}
			}


			l_vectorofvector_size_t.element_erase_element_at(l_index, 4);
			l_vectorofvector_size_t.element_push_back_array(l_index, &l_elements_slice);

			{
				VectorOfVector_Element<size_t> l_vector_element = l_vectorofvector_size_t.get(l_index);
				assert_true(l_vector_element.Header.Size == 8);
				for (loop(i, 0, 3))
				{
					assert_true(l_vector_element.Memory.get_rv(i) == l_initial_elements[i]);
				}

				assert_true(l_vector_element.Memory.get_rv(3) == l_elements[0]);
				assert_true(l_vector_element.Memory.get_rv(4) == l_elements[2]);
				assert_true(l_vector_element.Memory.get_rv(5) == l_elements[0]);
				assert_true(l_vector_element.Memory.get_rv(6) == l_elements[1]);
				assert_true(l_vector_element.Memory.get_rv(7) == l_elements[2]);
			}

			l_vectorofvector_size_t.element_clear(l_index);
			l_vectorofvector_size_t.element_push_back_array(l_index, &l_elements_slice);
			{
				VectorOfVector_Element<size_t> l_vector_element = l_vectorofvector_size_t.get(l_index);
				assert_true(l_vector_element.Header.Size == 3);
				assert_true(l_vector_element.Header.Capacity == 8);
				assert_true(l_vector_element.Memory.get_rv(0) == l_elements[0]);
				assert_true(l_vector_element.Memory.get_rv(1) == l_elements[1]);
				assert_true(l_vector_element.Memory.get_rv(2) == l_elements[2]);

			}
		}

		{
			l_vectorofvector_size_t.free();
		}

	};

	inline void poolofvector_test()
	{
		PoolOfVector<size_t> l_pool_of_vector = PoolOfVector<size_t>::allocate_default();

		// poolofvector_alloc_vector poolofvector_element_push_back_element poolofvector_release_vector
		{
			PoolOfVectorToken<size_t> l_vector_0 = l_pool_of_vector.alloc_vector();

			size_t l_element = 100;
			l_pool_of_vector.element_push_back_element(&l_vector_0, &l_element);

			VectorOfVector_Element<size_t> l_vector_mem = l_pool_of_vector.get_vector(&l_vector_0);
			assert_true(l_vector_mem.Header.Size == 1);
			assert_true(l_vector_mem.Memory.get_rv(0) == l_element);

			l_pool_of_vector.release_vector(&l_vector_0);

			PoolOfVectorToken<size_t> l_vector_0_new = l_pool_of_vector.alloc_vector();
			assert_true(l_vector_0_new.tok == l_vector_0.tok);
			l_vector_mem = l_pool_of_vector.get_vector(&l_vector_0);
			assert_true(l_vector_mem.Header.Size == 0);
		}

		// poolofvector_alloc_vector_with_values
		{
			size_t l_elements[3] = { 100,200,300 };
			Slice<size_t> l_elements_slice = Slice<size_t>::build_memory_elementnb(l_elements, 3);
			PoolOfVectorToken<size_t> l_vector_0 = l_pool_of_vector.alloc_vector_with_values(&l_elements_slice);

			VectorOfVector_Element<size_t> l_vector_mem = l_pool_of_vector.get_vector(&l_vector_0);
			assert_true(l_vector_mem.Header.Size == 3);
			for (loop(i, 0, 3))
			{
				assert_true(l_vector_mem.Memory.get_rv(i) == l_elements[i]);
			}
		}

		l_pool_of_vector.free();
	};

	inline void ntree_test()
	{
		NTree<size_t> l_size_t_tree = NTree<size_t>::allocate_default();

		Token<size_t> l_root = l_size_t_tree.push_root_value(cast(size_t, 0));
		l_size_t_tree.push_value(cast(size_t, 1), &l_root);
		Token<size_t> l_2_node = l_size_t_tree.push_value(cast(size_t, 2), &l_root);
		Token<size_t> l_3_node = l_size_t_tree.push_value(cast(size_t, 3), &l_root);

		l_size_t_tree.push_value(cast(size_t, 4), &l_2_node);
		l_size_t_tree.push_value(cast(size_t, 5), &l_2_node);

		Token<size_t> l_6_node = l_size_t_tree.push_value(cast(size_t, 6), &l_3_node);

		{
			assert_true(l_size_t_tree.Memory.get_size() == 7);
			assert_true(l_size_t_tree.Indices.get_size() == 7);

			// testing the root
			{
				NTree<size_t>::Resolve l_root_element = l_size_t_tree.get(&l_root);
				assert_true((*l_root_element.Element) == 0);
				assert_true(l_root_element.Node->parent.tok == -1);
				assert_true(l_root_element.Node->index.tok == 0);
				assert_true(l_root_element.Node->childs.tok != -1);

				Slice<Token(NTreeNode)> l_childs_indices = l_size_t_tree.get_childs(&l_root_element.Node->childs);
				assert_true(l_childs_indices.Size == 3);
				for (loop(i, 0, l_childs_indices.Size))
				{
					assert_true(*l_size_t_tree.get_value(cast(Token(size_t)*, l_childs_indices.get(i))) == i + 1);
				}
			}

			// testing one leaf
			{
				NTree<size_t>::Resolve l_2_element = l_size_t_tree.get(&l_2_node);
				assert_true((*l_2_element.Element) == 2);
				assert_true(l_2_element.Node->parent.tok == 0);
				assert_true(l_2_element.Node->index.tok == 2);
				assert_true(l_2_element.Node->childs.tok != -1);

				Slice<Token(NTreeNode)> l_childs_indices = l_size_t_tree.get_childs(&l_2_element.Node->childs);
				assert_true(l_childs_indices.Size == 2);
				for (loop(i, 0, l_childs_indices.Size))
				{
					assert_true(*l_size_t_tree.get_value(cast(Token(size_t)*, l_childs_indices.get(i))) == i + 4);
				}
			}
		}

		// traversing test
		{
			size_t l_counter = 0;
			NTree<size_t>::Traverse l_tree_traverse = NTree<size_t>::Traverse::build_default(&l_size_t_tree);
			while (l_tree_traverse.step() != NTree<size_t>::Traverse::State::END)
			{
				NTree<size_t>::Resolve* l_node = l_tree_traverse.get_current_node();
				*(l_node->Element) += 1;
				l_counter += 1;
			}
			assert_true(l_counter == 7);

			assert_true(*l_size_t_tree.get_value(&l_root) == 1);
			assert_true(*l_size_t_tree.get_value(&l_2_node) == 3);
			assert_true(*l_size_t_tree.get_value(&l_3_node) == 4);
			assert_true(*l_size_t_tree.get_value(&l_6_node) == 7);
		}

		// removal test
		{
			l_size_t_tree.remove_node(token_cast_p(NTreeNode, &l_2_node));

			NTree<size_t>::Resolve l_root_node = l_size_t_tree.get(&l_root);
			Slice<Token(NTreeNode)> l_root_node_childs = l_size_t_tree.get_childs(&l_root_node.Node->childs);
			assert_true(l_root_node_childs.Size == 2);

			{
				size_t l_counter = 0;
				NTree<size_t>::Traverse l_tree_traverse = NTree<size_t>::Traverse::build_default(&l_size_t_tree);
				while (l_tree_traverse.step() != NTree<size_t>::Traverse::State::END)
				{
					NTree<size_t>::Resolve* l_node = l_tree_traverse.get_current_node();
					*(l_node->Element) += 1;
					l_counter += 1;
				}
				assert_true(l_counter == 4);
			}
		}

		l_size_t_tree.free();
	};

}

int main()
{
	v2::span_test();
	v2::vector_test();
	v2::pool_test();
	v2::varyingvector_test();
	v2::vectorofvector_test();
	v2::poolofvector_test();
	v2::ntree_test();
}
