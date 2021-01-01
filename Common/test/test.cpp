#include "Common/Container/vector.hpp"
#include "Common/Memory/heap.hpp"
#include "Common/Container/varying_pool.hpp"
#include "Common/Container/pool_of_vector.hpp"
#include "Common/Container/varying_vector.hpp"

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



	VaryingPool l_varying_pool;
	l_varying_pool.allocate(0, 0);

	com::TPoolToken<VaryingPool_Header<SpacializedHeaderEmpty>> l_sizet = l_varying_pool.allocate_element<size_t>(SpacializedHeaderEmpty(), 20);
	com::TPoolToken<VaryingPool_Header<SpacializedHeaderEmpty>> l_sizet_1 = l_varying_pool.allocate_element<short int>(SpacializedHeaderEmpty(), 1);

	l_varying_pool.release_element(l_sizet_1);
	l_varying_pool.allocate_element<float>(SpacializedHeaderEmpty(), 2.0f);

	size_t* l_sizet_ptr = l_varying_pool.get_element<size_t>(l_sizet);

	l_varying_pool.free();


	PoolOfVector<int> l_int_pool_of_vector;
	l_int_pool_of_vector.allocate(0, 0);
	
	TPoolOfVectorToken<int> l_header = l_int_pool_of_vector.allocate_vector(1);
	
	for (int i = 0; i < 10; i++)
	{
		l_int_pool_of_vector.vector_push_back(l_header, i);
	}

	int& l_i = l_int_pool_of_vector.vector_get(l_header, 7);
	
	l_int_pool_of_vector.free_vector(l_header);

	l_int_pool_of_vector.free();

	size_t l_st = 1895;
	size_t l_st2 = 1896;

	VaryingVector2 l_v2; l_v2.allocate(0, 0);
	l_v2.push_back((char*)&l_st, sizeof(size_t));
	l_v2.push_back((char*)&l_st2, sizeof(size_t));


	l_v2.erase_at(0, 1);

	for (size_t i = 0; i < l_v2.size(); i++)
	{
		size_t* l_gget = (size_t*)&l_v2.get(i);
		int zd = 0;
	}

	l_v2.free();
}