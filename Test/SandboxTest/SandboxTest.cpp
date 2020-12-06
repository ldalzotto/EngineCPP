

#include "Common/Functional/Hash.hpp"

struct HashClassTest
{
	inline static const size_t test = Hash<ConstString>::hash("TEST");
};

int main(int argc, char** argv)
{
	size_t l_never_hash = Hash<ConstString>::hash("Never");
	size_t l_never_hash_2 = Hash<StringSlice>::hash(StringSlice("Never"));
	/*
	const ConstString l_st = ConstString("hello");
	size_t l_zd = l_st.get_size();
	l_st.length();
	*/

	size_t l_hashed = 3456789;
	switch (l_hashed)
	{
	case HashClassTest::test:
	{

	};
	}

}