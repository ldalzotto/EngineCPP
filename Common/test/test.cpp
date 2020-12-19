#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"

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
}