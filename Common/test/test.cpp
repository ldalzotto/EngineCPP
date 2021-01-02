
#include "Common/Container/nested_vector.hpp"

int main()
{
	size_t l_element_9 = 9;
	size_t l_element_12 = 12;

	TVectorOfVector<size_t> l_vv;
	l_vv.allocate(0, 0);

	TNestedVector<size_t> l_v0 = l_vv.push_back_vector();

	l_vv.nested_vector_push_back(l_v0, 9);
	l_vv.nested_vector_push_back(l_v0, 13);

	TNestedVector<size_t> l_v1 = l_vv.push_back_vector();

	l_vv.nested_vector_push_back(l_v1, 12);

	l_vv.nested_vector_erase_at(l_v0, 0, 1);

	size_t& l_st = l_vv.nested_vector_get(l_v0, 0);
	l_st = l_vv.nested_vector_get(l_v1, 0);

	

	l_vv.free();

}