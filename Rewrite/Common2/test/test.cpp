
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
		Span<uimax> l_span_sizet = Span<uimax>::build(NULL, 0);

		//When resizing the span, new memory is allocated
		{
			uimax l_new_capacity = 10;
			l_span_sizet.resize(10);
			assert_true(l_span_sizet.Capacity == l_new_capacity);
			assert_true(l_span_sizet.Memory != NULL);
		}

		//When freeing the span, it's structure is resetted
		{
			l_span_sizet.free();
			assert_span_unitialized(&l_span_sizet);
		}

		//Move memory
		{
			l_span_sizet.resize(10);

			l_span_sizet.get(0) = 3;
			l_span_sizet.get(1) = 5;
			l_span_sizet.get(3) = 10;

			l_span_sizet.move_memory_down(1, 3, 1);
			assert_true(l_span_sizet.get(4) == 10);

			l_span_sizet.move_memory_up(1, 4, 2);
			assert_true(l_span_sizet.get(2) == 10);

			assert_true(l_span_sizet.get(0) == 3);
			assert_true(l_span_sizet.get(1) == 5);

			l_span_sizet.move_memory_up(2, 2, 2);

			assert_true(l_span_sizet.get(0) == 10);
			assert_true(l_span_sizet.get(1) == 10);
		}
	};

	inline void vector_test()
	{
		Vector<uimax> l_vector_sizet = Vector<uimax>::build((uimax*)NULL, 0);

		// vector_push_back_array
		{
			uimax l_old_size = l_vector_sizet.Size;
			uimax l_elements[5] = { 0,1,2,3,4 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);

			l_vector_sizet.push_back_array(l_elements_slice);
			assert_true(l_vector_sizet.Size == l_old_size + 5);
			for (loop(i, l_old_size, l_vector_sizet.Size))
			{
				assert_true(l_vector_sizet.get(i) == l_elements[i - l_old_size]);
			}
		}

		// vector_push_back_element
		{
			uimax l_old_size = l_vector_sizet.Size;
			uimax l_element = 25;
			l_vector_sizet.push_back_element(l_element);
			assert_true(l_vector_sizet.Size == l_old_size + 1);
			assert_true(l_vector_sizet.get(l_vector_sizet.Size - 1) == l_element);
		}

		// vector_insert_array_at
		{
			uimax l_old_size = l_vector_sizet.Size;
			uimax l_elements[5] = { 0,1,2,3,4 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 5);
			l_vector_sizet.insert_array_at(l_elements_slice, 0);
			assert_true(l_vector_sizet.Size == l_old_size + 5);
			for (loop_int16(i, 0, 5))
			{
				assert_true((l_vector_sizet.get(i)) == (uimax)i);
			}

			l_vector_sizet.insert_array_at(l_elements_slice, 3);
			assert_true(l_vector_sizet.Size == l_old_size + 10);
			for (loop_int16(i, 0, 3))
			{
				assert_true((l_vector_sizet.get(i)) == l_elements[i]);
			}
			//Middle insertion
			for (loop_int16(i, 3, 8))
			{
				assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 3)]);
			}
			for (loop_int16(i, 8, 10))
			{
				assert_true((l_vector_sizet.get(i)) == l_elements[i - cast(uimax, 5)]);
			}
		}

		// vector_insert_element_at
		{
			uimax l_element = 20;
			uimax l_old_size = l_vector_sizet.Size;

			l_vector_sizet.insert_element_at(l_element, 7);
			assert_true(l_vector_sizet.get(7) == l_element);
			assert_true(l_vector_sizet.Size == l_old_size + 1);

			l_vector_sizet.insert_element_at(cast(uimax, 20), 9);
		}


		// vector_erase_element_at
		{
			uimax l_old_size = l_vector_sizet.Size;
			uimax l_erase_index = 1;
			uimax l_element_after = l_vector_sizet.get(l_erase_index + 1);
			l_vector_sizet.erase_element_at(1);
			assert_true(l_vector_sizet.Size == l_old_size - 1);
			assert_true(l_vector_sizet.get(1) == l_element_after);
		}


		// vector_erase_array_at
		{
			uimax l_old_size = l_vector_sizet.Size;
			uimax l_erase_begin_index = 3;
			const uimax l_erase_nb = 6;
			const uimax l_old_element_check_nb = 3;

			uimax l_old_values[l_old_element_check_nb];
			for (loop(i, l_erase_begin_index + l_erase_nb, (l_erase_begin_index + l_erase_nb) + l_old_element_check_nb))
			{
				l_old_values[i - (l_erase_begin_index + l_erase_nb)] = l_vector_sizet.get(i);
			}

			l_vector_sizet.erase_array_at(l_erase_begin_index, l_erase_nb);

			assert_true(l_vector_sizet.Size == l_old_size - l_erase_nb);
			for (loop(i, 0, l_old_element_check_nb))
			{
				assert_true(l_vector_sizet.get(l_erase_begin_index + i) == l_old_values[i]);
			}
		}

		// vector_pop_back
		{
			uimax l_old_size = l_vector_sizet.Size;
			l_vector_sizet.pop_back();
			assert_true(l_vector_sizet.Size == l_old_size - 1);
		}

		// vector_pop_back_array
		{
			uimax l_old_size = l_vector_sizet.Size;
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
		Pool<uimax> l_pool_sizet = Pool<uimax>::allocate(10);

		{
			assert_true(l_pool_sizet.get_memory() != NULL);
			assert_true(l_pool_sizet.get_capacity() == 10);
			assert_true(l_pool_sizet.get_size() == 0);
		}

		// pool_alloc_element - allocate new element
		{
			assert_true(l_pool_sizet.get_free_size() == 0);

			uimax l_element = 3;
			Token(uimax) l_token = l_pool_sizet.alloc_element(l_element);

			assert_true(tk_v(l_token) == 0);
			assert_true(l_pool_sizet.get(l_token) == l_element);
		}

		// pool_release_element - release elements
		{
			Token(uimax) l_token = Token(uimax) { 0 };
			l_pool_sizet.release_element(l_token);

			// memory is not deallocated
			assert_true(l_pool_sizet.get_size() == 1);
		}

		// pool_alloc_element - allocating an element while there is free slots
		{
			uimax l_element = 4;
			Token(uimax) l_token = l_pool_sizet.alloc_element(l_element);

			l_pool_sizet.alloc_element(cast(uimax, 10));
			l_pool_sizet.release_element_1v(
				l_pool_sizet.alloc_element(cast(uimax, 10))
			);
			l_pool_sizet.alloc_element(cast(uimax, 10));

			assert_true(tk_v(l_token) == 0);
			assert_true(l_pool_sizet.get(l_token) == l_element);
		}

		for (pool_loop(&l_pool_sizet, i))
		{
			l_pool_sizet.get(Token(uimax) { i });
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

			const int8* l_10_element = "abcdefhikl";
			Slice<int8> l_slice = Slice<int8>::build_memory_elementnb((int8*)l_10_element, 10);
			l_varyingvector.push_back(l_slice);

			assert_true(l_varyingvector.get_size() == 1);
			Slice<int8> l_element_0 = l_varyingvector.get(0);
			assert_true(l_element_0.Size == 10);
			assert_true(slice_memcompare_element(l_slice, l_element_0));
		}

		// varyingvector_push_back_element
		{
			uimax l_element = 20;
			l_varyingvector.push_back_element(l_element);

			uimax l_inserted_index = l_varyingvector.get_size() - 1;
			Slice<int8> l_element_inserted = l_varyingvector.get(l_inserted_index);

			assert_true(l_element_inserted.Size == sizeof(uimax));
			assert_true(memory_compare(cast(const int8*, &l_element), l_element_inserted.Begin, l_element_inserted.Size));

			Slice<uimax> l_casted_slice = slice_cast<uimax>(l_element_inserted);
			assert_true(l_casted_slice.Size == 1);

		}

		// varyingvector_pop_back
		{

			uimax l_old_size = l_varyingvector.get_size();
			l_varyingvector.pop_back();
			assert_true(l_varyingvector.get_size() == (l_old_size - 1));
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_erase_element_at
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element(cast(uimax, i));
			}

			assert_true(l_varyingvector.get_size() == 5);
			l_varyingvector.erase_element_at(2);
			assert_true(l_varyingvector.get_size() == 4);

			assert_true(*l_varyingvector.get_element<uimax>(2).Begin == 3);
			assert_true(*l_varyingvector.get_element<uimax>(3).Begin == 4);
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_erase_array_at
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element(cast(uimax, i));
			}

			assert_true(l_varyingvector.get_size() == 5);
			l_varyingvector.erase_array_at(2, 2);
			assert_true(l_varyingvector.get_size() == 3);

			assert_true(*l_varyingvector.get_element<uimax>(2).Begin == 4);
		}

		l_varyingvector.free();
		l_varyingvector = VaryingVector::allocate_default();

		// varyingvector_element_expand_with_value varyingvector_element_shrink
		{
			for (loop(i, 0, 5))
			{
				l_varyingvector.push_back_element(cast(uimax, i));
			}

			uimax l_inserset_number = 30;
			Slice<int8> l_expansion_slice = Slice<uimax>::build_asint8_memory_elementnb(&l_inserset_number, 1);
			l_varyingvector.element_expand_with_value(2, l_expansion_slice);

			Slice<uimax> l_sizet_element_2 = slice_cast<uimax>(l_varyingvector.get(2));
			assert_true(l_sizet_element_2.Size == 2);
			assert_true(l_sizet_element_2.get(1) == l_inserset_number);

			{
				uimax* l_sizet_element_3 = slice_cast_singleelement<uimax>(l_varyingvector.get(3));
				assert_true(*l_sizet_element_3 == 3);
			}

			l_varyingvector.element_shrink(2, sizeof(uimax));
			l_sizet_element_2 = slice_cast<uimax>(l_varyingvector.get(2));
			assert_true(l_sizet_element_2.Size == 1);
			assert_true(l_sizet_element_2.get(0) == 2);

			{
				uimax* l_sizet_element_3 = slice_cast_singleelement<uimax>(l_varyingvector.get(3));
				assert_true(*l_sizet_element_3 == 3);
			}
		}

		// varyingvector_element_writeto
		{
			uimax l_element_0 = 10;
			uimax l_element_1 = 20;
			uimax l_element_2 = 30;

			l_varyingvector.element_expand(2, sizeof(uimax) * 3);
			l_varyingvector.element_writeto(2, 0, Slice<uimax>::build_asint8_memory_singleelement(&l_element_0));
			l_varyingvector.element_writeto(2, 2 * sizeof(uimax), Slice<uimax>::build_asint8_memory_singleelement(&l_element_2));
			l_varyingvector.element_writeto(2, 1 * sizeof(uimax), Slice<uimax>::build_asint8_memory_singleelement(&l_element_1));


			Slice<int8> l_varyingvector_element_2 = l_varyingvector.get(2);
			assert_true(*cast(uimax*, l_varyingvector_element_2.Begin) == l_element_0);
			assert_true(*cast(uimax*, l_varyingvector_element_2.slide_rv(sizeof(uimax)).Begin) == l_element_1);
			assert_true(*cast(uimax*, l_varyingvector_element_2.slide_rv(2 * sizeof(uimax)).Begin) == l_element_2);
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
		VectorOfVector<uimax> l_vectorofvector_uimax = VectorOfVector<uimax>::allocate_default();

		// vectorofvector_push_back vectorofvector_push_back_element
		{
			Span<uimax> l_sizets = Span<uimax>::allocate(10);
			for (loop(i, 0, l_sizets.Capacity))
			{
				l_sizets.slice.get(i) = i;
			}

			l_vectorofvector_uimax.push_back();

			l_vectorofvector_uimax.push_back_element(l_sizets.slice);
			uimax l_requested_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;
			Slice<uimax> l_element = l_vectorofvector_uimax.get(l_requested_index);

			l_vectorofvector_uimax.push_back();

			assert_true(l_vectorofvector_uimax.get_vectorheader(l_requested_index)->Capacity == l_sizets.Capacity);
			for (loop(i, 0, l_sizets.Capacity))
			{
				assert_true(l_element.get(i) == i);
			}

			l_sizets.free();
		}

		// vectorofvector_element_push_back_element
		{
			uimax l_index;
			for (loop(i, 0, 2))
			{
				l_vectorofvector_uimax.push_back();

				uimax l_element = 30;
				l_index = l_vectorofvector_uimax.varying_vector.get_size() - 2;
				l_vectorofvector_uimax.element_push_back_element(l_index, l_element);
				Slice<uimax> l_element_nested = l_vectorofvector_uimax.get(l_index);
				assert_true(l_element_nested.Size == 1);
				assert_true(l_element_nested.get(0) == l_element);

				l_element = 35;
				l_vectorofvector_uimax.element_clear(l_index);
				l_vectorofvector_uimax.element_push_back_element(l_index, l_element);
				assert_true(l_element_nested.Size == 1);
				assert_true(l_element_nested.get(0) == l_element);
			}
		}

		// vectorofvector_element_insert_element_at
		{
			uimax l_elements[3] = { 100,120,140 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
			l_vectorofvector_uimax.push_back_element(l_elements_slice);
			uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;

			uimax l_inserted_element = 200;
			l_vectorofvector_uimax.element_insert_element_at(l_index, 1, l_inserted_element);

			Slice<uimax> l_vector = l_vectorofvector_uimax.get(l_index);
			assert_true(l_vector.Size == 4);
			assert_true(l_vector.get(0) == l_elements[0]);
			assert_true(l_vector.get(1) == l_inserted_element);
			assert_true(l_vector.get(2) == l_elements[1]);
			assert_true(l_vector.get(3) == l_elements[2]);
		}

		// vectorofvector_element_erase_element_at
		{
			uimax l_elements[3] = { 100,120,140 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
			l_vectorofvector_uimax.push_back_element(l_elements_slice);

			// uimax l_inserted_element = 200;
			uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;
			l_vectorofvector_uimax.element_erase_element_at(l_index, 1);
			Slice<uimax> l_vector = l_vectorofvector_uimax.get(l_index);
			assert_true(l_vector.Size == 2);
			assert_true(l_vector.get(0) == l_elements[0]);
			assert_true(l_vector.get(1) == l_elements[2]);
		}

		// vectorofvector_element_push_back_array
		{
			uimax l_initial_elements[3] = { 1,2,3 };
			{
				Slice<uimax> l_initial_elements_slice = Slice<uimax>::build_memory_elementnb(l_initial_elements, 3);
				l_vectorofvector_uimax.push_back_element(l_initial_elements_slice);
			}

			uimax l_index = l_vectorofvector_uimax.varying_vector.get_size() - 1;

			uimax l_elements[3] = { 100,120,140 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);

			uimax l_old_size = 0;
			{
				Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
				l_old_size = l_vector_element.Size;
			}

			l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);

			{
				Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
				assert_true(l_vector_element.Size == l_old_size + 3);
				for (loop(i, 0, 3))
				{
					assert_true(l_vector_element.get(i) == l_initial_elements[i]);
				}
				for (loop(i, l_old_size, l_old_size + 3))
				{
					assert_true(l_vector_element.get(i) == l_elements[i - l_old_size]);
				}
			}


			l_vectorofvector_uimax.element_erase_element_at(l_index, 4);
			l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);

			{
				Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
				assert_true(l_vector_element.Size == 8);
				for (loop(i, 0, 3))
				{
					assert_true(l_vector_element.get(i) == l_initial_elements[i]);
				}

				assert_true(l_vector_element.get(3) == l_elements[0]);
				assert_true(l_vector_element.get(4) == l_elements[2]);
				assert_true(l_vector_element.get(5) == l_elements[0]);
				assert_true(l_vector_element.get(6) == l_elements[1]);
				assert_true(l_vector_element.get(7) == l_elements[2]);
			}

			l_vectorofvector_uimax.element_clear(l_index);
			l_vectorofvector_uimax.element_push_back_array(l_index, l_elements_slice);
			{
				Slice<uimax> l_vector_element = l_vectorofvector_uimax.get(l_index);
				assert_true(l_vector_element.Size == 3);
				assert_true(l_vectorofvector_uimax.get_vectorheader(l_index)->Capacity == 8);
				assert_true(l_vector_element.get(0) == l_elements[0]);
				assert_true(l_vector_element.get(1) == l_elements[1]);
				assert_true(l_vector_element.get(2) == l_elements[2]);

			}
		}

		{
			l_vectorofvector_uimax.free();
		}

	};

	inline void poolofvector_test()
	{
		PoolOfVector<uimax> l_pool_of_vector = PoolOfVector<uimax>::allocate_default();

		// poolofvector_alloc_vector poolofvector_element_push_back_element poolofvector_release_vector
		{
			PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector();

			uimax l_element = 100;
			l_pool_of_vector.element_push_back_element(l_vector_0, l_element);

			Slice<uimax> l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
			assert_true(l_vector_mem.Size == 1);
			assert_true(l_vector_mem.get(0) == l_element);

			l_pool_of_vector.release_vector(l_vector_0);

			PoolOfVectorToken<uimax> l_vector_0_new = l_pool_of_vector.alloc_vector();
			assert_true(tk_v(l_vector_0_new) == tk_v(l_vector_0));
			l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
			assert_true(l_vector_mem.Size == 0);
		}

		// poolofvector_alloc_vector_with_values
		{
			uimax l_elements[3] = { 100,200,300 };
			Slice<uimax> l_elements_slice = Slice<uimax>::build_memory_elementnb(l_elements, 3);
			PoolOfVectorToken<uimax> l_vector_0 = l_pool_of_vector.alloc_vector_with_values(l_elements_slice);

			Slice<uimax> l_vector_mem = l_pool_of_vector.get_vector(l_vector_0);
			assert_true(l_vector_mem.Size == 3);
			for (loop(i, 0, 3))
			{
				assert_true(l_vector_mem.get(i) == l_elements[i]);
			}
		}

		l_pool_of_vector.free();
	};

	inline void ntree_test()
	{
		NTree<uimax> l_uimax_tree = NTree<uimax>::allocate_default();

		Token(uimax) l_root = l_uimax_tree.push_root_value(cast(uimax, 0));
		l_uimax_tree.push_value(cast(uimax, 1), l_root);
		Token(uimax) l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
		Token(uimax) l_3_node = l_uimax_tree.push_value(cast(uimax, 3), l_root);

		l_uimax_tree.push_value(cast(uimax, 4), l_2_node);
		l_uimax_tree.push_value(cast(uimax, 5), l_2_node);

		Token(uimax) l_6_node = l_uimax_tree.push_value(cast(uimax, 6), l_3_node);

		{
			assert_true(l_uimax_tree.Memory.get_size() == 7);
			assert_true(l_uimax_tree.Indices.get_size() == 7);

			// testing the root
			{
				NTree<uimax>::Resolve l_root_element = l_uimax_tree.get(l_root);
				assert_true((*l_root_element.Element) == 0);
				assert_true(tk_v(l_root_element.Node->parent) == (token_t)-1);
				assert_true(tk_v(l_root_element.Node->index) == (token_t)0);
				assert_true(tk_v(l_root_element.Node->childs) != (token_t)-1);

				Slice<Token(NTreeNode)> l_childs_indices = l_uimax_tree.get_childs(l_root_element.Node->childs);
				assert_true(l_childs_indices.Size == 3);
				for (loop(i, 0, l_childs_indices.Size))
				{
					assert_true(l_uimax_tree.get_value(tk_bf(uimax, l_childs_indices.get(i))) == i + 1);
				}
			}

			// testing one leaf
			{
				NTree<uimax>::Resolve l_2_element = l_uimax_tree.get(l_2_node);
				assert_true((*l_2_element.Element) == 2);
				assert_true(tk_v(l_2_element.Node->parent) == (token_t)0);
				assert_true(tk_v(l_2_element.Node->index) == (token_t)2);
				assert_true(tk_v(l_2_element.Node->childs) != (token_t)-1);

				Slice<Token(NTreeNode)> l_childs_indices = l_uimax_tree.get_childs(l_2_element.Node->childs);
				assert_true(l_childs_indices.Size == 2);
				for (loop(i, 0, l_childs_indices.Size))
				{
					assert_true(l_uimax_tree.get_value(tk_bf(uimax, l_childs_indices.get(i))) == i + 4);
				}
			}
		}

		// traversing test
		{
			uimax l_counter = 0;

			tree_traverse2_stateful_begin(uimax, uimax * l_counter, CounterForEach);
			*this->l_counter += 1;
			*(p_node.Element) += 1;
			tree_traverse2_stateful_end(uimax, &l_uimax_tree, Token(NTreeNode){0}, & l_counter, CounterForEach);

			assert_true(l_counter == 7);

			assert_true(l_uimax_tree.get_value(l_root) == 1);
			assert_true(l_uimax_tree.get_value(l_2_node) == 3);
			assert_true(l_uimax_tree.get_value(l_3_node) == 4);
			assert_true(l_uimax_tree.get_value(l_6_node) == 7);
		}

		// removal test
		{
			l_uimax_tree.remove_node_recursively(tk_bf(NTreeNode, l_2_node));

			NTree<uimax>::Resolve l_root_node = l_uimax_tree.get(l_root);
			Slice<Token(NTreeNode)> l_root_node_childs = l_uimax_tree.get_childs(l_root_node.Node->childs);
			assert_true(l_root_node_childs.Size == 2);

			{
				uimax l_counter = 0;
				tree_traverse2_stateful_begin(uimax, uimax * l_counter, TreeForeach);
				*this->l_counter += 1;
				*p_node.Element += 1;
				tree_traverse2_stateful_end(uimax, &l_uimax_tree, Token(NTreeNode){0}, & l_counter, TreeForeach);

				assert_true(l_counter == 4);
			}
		}

		// add_child
		{
			l_2_node = l_uimax_tree.push_value(cast(uimax, 2), l_root);
			Token(uimax) l_2_1_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);
			Token(uimax) l_2_2_node = l_uimax_tree.push_value(cast(uimax, 3), l_2_node);

			assert_true(l_uimax_tree.add_child(l_3_node, l_2_2_node));

			Slice<Token(NTreeNode)> l_2_node_childs = l_uimax_tree.get_childs_from_node(tk_bf(NTreeNode, l_2_node));
			assert_true(l_2_node_childs.Size == 1);
			assert_true(tk_v(l_2_node_childs.get(0)) == tk_v(l_2_1_node));

			Slice<Token(NTreeNode)> l_3_node_childs = l_uimax_tree.get_childs_from_node(tk_bf(NTreeNode, l_3_node));
			assert_true(l_3_node_childs.Size == 2);
			assert_true(tk_v(l_3_node_childs.get(1)) == tk_v(l_2_2_node));

			assert_true(tk_v(l_uimax_tree.get(l_2_2_node).Node->parent) == tk_v(l_3_node));
		}

		l_uimax_tree.free();
	};

	inline void assert_heap_integrity(Heap* p_heap)
	{
		uimax l_calculated_size = 0;
		for (loop(i, 0, p_heap->AllocatedChunks.get_size()))
		{
			// Token(SliceIndex)* l_chunk = ;
			if (!p_heap->AllocatedChunks.is_element_free(Token(SliceIndex) { i }))
			{
				l_calculated_size += p_heap->AllocatedChunks.get(Token(SliceIndex) { i }).Size;
			};
		}

		for (loop(i, 0, p_heap->FreeChunks.Size))
		{
			l_calculated_size += p_heap->FreeChunks.get(i).Size;
		}

		assert_true(l_calculated_size == p_heap->Size);
	};

	inline void sort_test()
	{
		{
			uimax l_sizet_array[10] = { 10,9,8,2,7,4,10,35,9,4 };
			uimax l_sorted_sizet_array[10] = { 35,10,10,9,9,8,7,4,4,2 };
			Slice<uimax> l_slice = Slice<uimax>::build_memory_elementnb(l_sizet_array, 10);

			sort_linear2_begin(uimax, Tesss);
			return p_left < p_right;
			sort_linear2_end(l_slice, uimax, Tesss);

			assert_true(memcmp(l_sizet_array, l_sorted_sizet_array, sizeof(uimax) * 10) == 0);
		}
	};

	inline void heap_test()
	{
		uimax l_initial_heap_size = 20;
		Heap l_heap = Heap::allocate(l_initial_heap_size);
		assert_heap_integrity(&l_heap);

		{

			Heap::AllocatedElementReturn l_chunk_1;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(10, &l_chunk_1) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED);
			assert_true(l_heap.get(l_chunk_1.token)->Begin == 0);
			assert_true(l_heap.get(l_chunk_1.token)->Size == 10);
			assert_heap_integrity(&l_heap);


			Heap::AllocatedElementReturn l_chunk_0;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(5, &l_chunk_0) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED);
			assert_true(l_heap.get(l_chunk_0.token)->Begin == 10);
			assert_true(l_heap.get(l_chunk_0.token)->Size == 5);
			assert_heap_integrity(&l_heap);


			Heap::AllocatedElementReturn l_chunk_2;
			l_heap.allocate_element(5, &l_chunk_2);
			assert_heap_integrity(&l_heap);

			// Releasing elements
			l_heap.release_element(l_chunk_0.token);
			l_heap.release_element(l_chunk_2.token);
			assert_heap_integrity(&l_heap);

			// We try to allocate 10 but there is two chunks size 5 free next to each other
			assert_true(l_heap.allocate_element(10, &l_chunk_0) == Heap::AllocationState::ALLOCATED);
			assert_heap_integrity(&l_heap);


			// The heap is resized
			Heap::AllocatedElementReturn l_chunk_3;
			assert_true((Heap::AllocationState_t)l_heap.allocate_element(50, &l_chunk_3) & (Heap::AllocationState_t)Heap::AllocationState::ALLOCATED_AND_HEAP_RESIZED);
			assert_true(l_chunk_3.Offset == 20);
			assert_true(l_heap.get(l_chunk_3.token)->Size == 50);
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
		uimax l_initial_heap_size = 20 * sizeof(uimax);
		HeapMemory l_heap_memory = HeapMemory::allocate(l_initial_heap_size);

		uimax l_element = 10;
		Token(SliceIndex) l_sigle_sizet_chunk;

		// single allocation
		{
			l_sigle_sizet_chunk = l_heap_memory.allocate_element_typed<uimax>(&l_element);
			uimax* l_st = l_heap_memory.get_typed<uimax>(l_sigle_sizet_chunk);
			assert_true(*l_st == l_element);
		}

		// resize
		{
			uimax l_initial_heap_size = l_heap_memory.Memory.Capacity;
			l_heap_memory.allocate_empty_element(30 * sizeof(uimax));
			assert_true(l_heap_memory.Memory.Capacity != l_initial_heap_size);
			assert_true(l_heap_memory._Heap.Size != l_initial_heap_size);
			uimax* l_st = l_heap_memory.get_typed<uimax>(l_sigle_sizet_chunk);
			assert_true(*l_st == l_element);
		}


		l_heap_memory.free();
	};


	inline void string_test()
	{
		uimax l_initial_string_capacity = 20;
		String l_str = String::allocate(l_initial_string_capacity);

		assert_true(l_str.get(0) == (int8)NULL);
		assert_true(l_str.get_size() == 1);
		assert_true(l_str.get_int8_nb() == 0);

		// append
		{
			l_str.append(slice_int8_build_rawstr("ABC"));
			assert_true(l_str.get_int8_nb() == 3);
			assert_true(l_str.get(0) == 'A');
			assert_true(l_str.get(1) == 'B');
			assert_true(l_str.get(2) == 'C');
		}

		{
			l_str.insert_array_at(slice_int8_build_rawstr("DEA"), 2);
			assert_true(l_str.get_int8_nb() == 6);
			assert_true(l_str.get(2) == 'D');
			assert_true(l_str.get(3) == 'E');
			assert_true(l_str.get(4) == 'A');
		}

		// remove_int8s
		{
			l_str.remove_int8s('A');
			assert_true(l_str.get_int8_nb() == 4);
			assert_true(l_str.get(0) == 'B');
			assert_true(l_str.get(3) == 'C');
		}

		//to_slice
		{
			Slice<int8> l_slice = l_str.to_slice();
			assert_true(l_slice.Size == 4);
			assert_true(l_slice.get(0) == 'B');
			assert_true(l_slice.get(3) == 'C');
		}

		l_str.free();
		l_str = String::allocate(l_initial_string_capacity);
		l_str.append(slice_int8_build_rawstr("Don't Count Your Chickens Before They Hatch."));

		// find
		{
			uimax l_index;
			assert_true(l_str.to_slice().find(slice_int8_build_rawstr("efor"), &l_index) == 1);
			assert_true(l_index == 27);

			//no found
			l_index = 0;
			assert_true(l_str.to_slice().find(slice_int8_build_rawstr("eforc"), &l_index) == 0);
		}

		l_str.free();
	};

	inline void deserialize_test()
	{
		{
			const int8* l_json =
				"{"
				"\"local_position\":{"
				"\"x\":  \"16.550000\","
				"\"y\" : \"16.650000\","
				"\"z\" : \"16.750000\""
				"},"
				"\"local_position2\":{"
				"\"x\":\"  17.550000\","
				"\"y\" : \"17.650000\","
				"\"z\" : \"17.750000\""
				"},"
				"\"nodes\" : ["
				"{"
				"\"local_position\":{"
				"\"x\":\"  10.550000\","
				"\"y\" : \"10.650000\","
				"\"z\" : \"10.750000\""
				"}"
				"},"
				"{"
				"\"local_position\":{"
				"\"x\":\"  11.550000\","
				"\"y\" : \"11.650000\","
				"\"z\" : \"11.750000\""
				"}"
				"},"
				"{"
				"\"local_position\":{"
				"\"x\":\"  12.550000\","
				"\"y\" : \"12.650000\","
				"\"z\" : \"12.750000\""
				"}"
				"}"
				"]"
				"}";

			String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
			JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

			JSONDeserializer l_v3;
			l_deserialized.next_object("local_position", &l_v3);

			l_v3.next_field("x");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.550000f);
			l_v3.next_field("y");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.650000f);
			l_v3.next_field("z");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.750000f);

			l_deserialized.next_object("local_position2", &l_v3);


			l_v3.next_field("x");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.550000f);
			l_v3.next_field("y");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.650000f);
			l_v3.next_field("z");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 17.750000f);

			JSONDeserializer l_array = JSONDeserializer::allocate_default(), l_object = JSONDeserializer::allocate_default();
			l_deserialized.next_array("nodes", &l_array);


			float32 l_delta = 0.0f;
			while (l_array.next_array_object(&l_object))
			{
				l_object.next_object("local_position", &l_v3);
				l_v3.next_field("x");
				assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.550000f + l_delta);
				l_v3.next_field("y");
				assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.650000f + l_delta);
				l_v3.next_field("z");
				assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 10.750000f + l_delta);
				l_delta += 1;
			}

			l_deserialized.free();
		}


		// empty array
		{
			const int8* l_json =
				"{"
				"\"nodes\":[]}";


			String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
			JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

			JSONDeserializer l_array = JSONDeserializer::allocate_default(), l_object = JSONDeserializer::allocate_default();
			l_deserialized.next_array("nodes", &l_array);
			l_array.next_array_object(&l_object);
			l_deserialized.free();
		}

		// missed field
		{
			const int8* l_json =
				"{"
				"\"local_position\":{"
				"\"x\":\"16.506252\","
				"\"y\" : \"16.604988\","
				"\"z\" : \"16.705424\""
				"}";

			String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
			JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);

			JSONDeserializer l_v3;
			l_deserialized.next_object("local_position", &l_v3);
			l_v3.next_field("x");
			l_v3.next_field("y");
			l_v3.next_field("zz");
			l_v3.next_field("z");
			assert_true(FromString::afloat32(l_v3.get_currentfield().value) == 16.705424f);
		}

		// only fields
		{
			const int8* l_json =
				"{"
				"\"x\":\"16.506252\","
				"\"y\" : \"16.604988\","
				"\"z\" : \"16.705424\""
				"}";


			String l_json_str = String::allocate_elements(slice_int8_build_rawstr(l_json));
			JSONDeserializer l_deserialized = JSONDeserializer::start(l_json_str);
			l_deserialized.next_field("x");
			assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.506252f);
			l_deserialized.next_field("y");
			assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.604988f);
			l_deserialized.next_field("z");
			assert_true(FromString::afloat32(l_deserialized.get_currentfield().value) == 16.705424f);
		}
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
	v2::string_test();
	v2::deserialize_test();
}