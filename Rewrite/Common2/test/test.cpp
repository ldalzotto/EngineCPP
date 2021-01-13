
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
			size_t l_counter_2 = 1;

			tree_traverse2_stateful_begin(size_t, size_t * l_counter, CounterForEach);
			*this->l_counter += 1;
			*(p_node->Element) += 1;
			tree_traverse2_stateful_end(size_t, &l_size_t_tree, Token(NTreeNode){0}, & l_counter, CounterForEach);

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
				tree_traverse2_stateful_begin(size_t, size_t * l_counter, TreeForeach);
				*this->l_counter += 1;
				*p_node->Element += 1;
				tree_traverse2_stateful_end(size_t, &l_size_t_tree, Token(NTreeNode){0}, & l_counter, TreeForeach);

				assert_true(l_counter == 4);
			}
		}

		l_size_t_tree.free();
	};

	inline void assert_heap_integrity(Heap* p_heap)
	{
		size_t l_calculated_size = 0;
		for (loop(i, 0, p_heap->AllocatedChunks.get_size()))
		{
			// Token(SliceIndex)* l_chunk = ;
			if (!p_heap->AllocatedChunks.is_element_free_1v(Token(SliceIndex) { i }))
			{
				l_calculated_size += p_heap->AllocatedChunks.get_1v(Token(SliceIndex) { i })->Size;
			};
		}

		for (loop(i, 0, p_heap->FreeChunks.Size))
		{
			l_calculated_size += p_heap->FreeChunks.get(i)->Size;
		}

		assert_true(l_calculated_size == p_heap->Size);
	};

	inline void sort_test()
	{
		{
			size_t l_sizet_array[10] = { 10,9,8,2,7,4,10,35,9,4 };
			size_t l_sorted_sizet_array[10] = { 35,10,10,9,9,8,7,4,4,2 };
			Slice<size_t> l_slice = Slice<size_t>::build_memory_elementnb(l_sizet_array, 10);

			sort_linear2_begin(size_t, Tesss);
			return *p_left < *p_right;
			sort_linear2_end(&l_slice, size_t, Tesss);

			assert_true(memcmp(l_sizet_array, l_sorted_sizet_array, sizeof(size_t) * 10) == 0);
		}
	};

	inline void heap_test()
	{
		size_t l_initial_heap_size = 20;
		Heap l_heap = Heap::allocate(l_initial_heap_size);
		assert_heap_integrity(&l_heap);

		{

			Heap::AllocatedElementReturn l_chunk_1;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(10, &l_chunk_1) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED);
			assert_true(l_heap.get(&l_chunk_1.token)->Begin == 0);
			assert_true(l_heap.get(&l_chunk_1.token)->Size == 10);
			assert_heap_integrity(&l_heap);


			Heap::AllocatedElementReturn l_chunk_0;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(5, &l_chunk_0) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED);
			assert_true(l_heap.get(&l_chunk_0.token)->Begin == 10);
			assert_true(l_heap.get(&l_chunk_0.token)->Size == 5);
			assert_heap_integrity(&l_heap);


			Heap::AllocatedElementReturn l_chunk_2;
			l_heap.allocate_element(5, &l_chunk_2);
			assert_heap_integrity(&l_heap);

			// Releasing elements
			l_heap.release_element(&l_chunk_0.token);
			l_heap.release_element(&l_chunk_2.token);
			assert_heap_integrity(&l_heap);

			// We try to allocate 10 but there is two chunks size 5 free next to each other
			assert_true(l_heap.allocate_element(10, &l_chunk_0) == Heap::AllocationState::ALLOCATED);
			assert_heap_integrity(&l_heap);


			// The heap is resized
			Heap::AllocatedElementReturn l_chunk_3;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(50, &l_chunk_3) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED_AND_HEAP_RESIZED);
			assert_true(l_chunk_3.Offset == 20);
			assert_true(l_heap.get(&l_chunk_3.token)->Size == 50);
			assert_true(l_heap.Size > l_initial_heap_size);
			assert_heap_integrity(&l_heap);


		}

		l_heap.free();

		l_heap = Heap::allocate(l_initial_heap_size);
		assert_heap_integrity(&l_heap);
		{

			Heap::AllocatedElementReturn l_allocated_chunk;
			assert_true(l_heap.allocate_element_with_alignment(1, 5, &l_allocated_chunk) == Heap::AllocationState::ALLOCATED);
			assert_heap_integrity(&l_heap);
			assert_true(l_heap.allocate_element_with_alignment(7, 5, &l_allocated_chunk) == Heap::AllocationState::ALLOCATED);
			assert_heap_integrity(&l_heap);
			assert_true(l_allocated_chunk.Offset == 5);
			assert_true(l_heap.allocate_element_with_alignment(3, 7, &l_allocated_chunk) == Heap::AllocationState::ALLOCATED);
			assert_heap_integrity(&l_heap);
			assert_true(l_allocated_chunk.Offset == 14);

			assert_true(l_heap.Size == l_initial_heap_size);
		}
	};

	inline void heap_memory_test()
	{
		size_t l_initial_heap_size = 20 * sizeof(size_t);
		HeapMemory l_heap_memory = HeapMemory::allocate(l_initial_heap_size);

		size_t l_element = 10;
		Token(SliceIndex) l_sigle_sizet_chunk;

		// single allocation
		{
			l_sigle_sizet_chunk = l_heap_memory.allocate_element_typed<size_t>(&l_element);
			size_t* l_st = l_heap_memory.get_typed<size_t>(&l_sigle_sizet_chunk);
			assert_true(*l_st == l_element);
		}

		// resize
		{
			size_t l_initial_heap_size = l_heap_memory.Memory.Capacity;
			Token(SliceIndex) l_chunk = l_heap_memory.allocate_empty_element(30 * sizeof(size_t));
			assert_true(l_heap_memory.Memory.Capacity != l_initial_heap_size);
			assert_true(l_heap_memory._Heap.Size != l_initial_heap_size);
			size_t* l_st = l_heap_memory.get_typed<size_t>(&l_sigle_sizet_chunk);
			assert_true(*l_st == l_element);
		}


		l_heap_memory.free();
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
	v2::sort_test();
	v2::heap_test();
	v2::heap_memory_test();
}
