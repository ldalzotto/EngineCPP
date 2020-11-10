#pragma once

#include <string>
#include "Common/Container/vector.hpp"

struct File
{
	inline static void read_bytes(const std::string& p_path, com::Vector<char>& out_file)
	{
		{
			std::ifstream l_stream(p_path, std::ios::binary | std::ios::in | std::ios::ate);
			if (l_stream.is_open())
			{
				size_t l_size = l_stream.tellg();
				l_stream.seekg(0, std::ios::beg);
				out_file.resize(l_size);
				out_file.Size = out_file.Capacity;
				l_stream.read((char*)out_file.Memory, l_size);
				l_stream.close();
			}
		}
	};
};