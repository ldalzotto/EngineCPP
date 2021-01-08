
#include "Common/Container/nested_vector.hpp"

int main()
{
	com::Vector<size_t> l_st;
	for (size_t i = 0; i < 1000000; i++)
	{
		l_st.insert_at(i, 0);
	}
}