#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"
#include "Common/Container/pool_v2.hpp"
#include "Common/Container/vector_of_vector.hpp"

#include <stdio.h>

int main()
{
	GeneralPurposeHeap2<GeneralPurposeHeap2_Times2Allocation> l_heapu;
	l_heapu.allocate(20);

	com::TPoolToken<GeneralPurposeHeapMemoryChunk> l_c1, l_c2, l_c3, l_c4, l_c5;
	l_heapu.allocate_element(6, &l_c1);
	l_heapu.allocate_element(6, &l_c2);
	l_heapu.allocate_element(6, &l_c3);
	l_heapu.allocate_element(6, &l_c4);
	l_heapu.allocate_element(6, &l_c5);

	l_heapu.release_element(l_c2);
	l_heapu.release_element(l_c5);
	l_heapu.release_element(l_c3);

	l_heapu.defragment();

	l_heapu.release_element(l_c4);
	l_heapu.defragment();

	l_heapu.dispose();

	Pool_v2<int> l_int_pool; pool_allocate(&l_int_pool);
	{
		int l_element = 10;
		TToken<int> l_token = pool_alloc_element(&l_int_pool, &l_element);
		int* l_memory = pool_get(&l_int_pool, &l_token);
		printf("%ld \n", *l_memory);
	}
	pool_free(&l_int_pool);


	struct ElementTest
	{
		int v1;
		int v2;

		inline static ElementTest build(int p_index) { return ElementTest{ p_index , p_index }; };
	};


	VectorOfVector l_vv = VectorOfVector<>::allocate(0);
	TToken<com::Vector<int, NoAllocator>> l_int_vec = l_vv.allocate_vector<int>(5);
	for (short int i = 0; i < 10; i++)
	{
		l_vv.vector_push_back(l_int_vec, (int)i);
	}

	TToken<com::Vector<ElementTest, NoAllocator>> l_test_vec = l_vv.allocate_vector<ElementTest>(0);
	for (short int i = 0; i < 10; i++)
	{
		l_vv.vector_push_back(l_test_vec, ElementTest::build((int)i));
	}

	int& l_value = l_vv.vector_get(l_int_vec, 3);
	ElementTest& l_element_test = l_vv.vector_get(l_test_vec, 8);
	l_vv.free();
}