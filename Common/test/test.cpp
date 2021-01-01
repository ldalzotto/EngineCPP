#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"
#include "Common/Container/pool_v2.hpp"

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
}