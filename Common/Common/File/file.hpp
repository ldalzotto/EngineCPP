#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include "Common/Include/platform_include.hpp"
#include "Common/Container/vector.hpp"
#include "Common/Container/string.hpp"

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

	template<class FindFileFilterFn>
	inline static void find_file(const StringSlice& p_path, com::Vector<String<>>& out_paths_buffer, bool p_recursive)
	{
		WIN32_FIND_DATA l_find_data;
		HANDLE l_find = INVALID_HANDLE_VALUE;

		String<> l_current_file_first;
		l_current_file_first.allocate(0);
		{
			l_current_file_first.append(p_path).append("*");
			l_find = FindFirstFile(l_current_file_first.Memory.Memory, &l_find_data);
		}
		l_current_file_first.free();

		while (l_find != INVALID_HANDLE_VALUE)
		{
			if (p_recursive && (l_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (p_recursive)
				{
					if (!StringSlice(".").equals(StringSlice(l_find_data.cFileName)) &&
						!StringSlice("..").equals(StringSlice(l_find_data.cFileName)))
					{
						String<> l_new_path;
						l_new_path.allocate(0);
						{
							l_new_path.append(p_path);
							l_new_path.append(l_find_data.cFileName);
							l_new_path.append("/");
							find_file<FindFileFilterFn>(l_new_path.toSlice(), out_paths_buffer, p_recursive);
						}
						l_new_path.free();
					}
				}

			}
			else
			{
				String<> l_file_path;
				l_file_path.allocate(0);
				{
					l_file_path.append(p_path);
					l_file_path.append(l_find_data.cFileName);
					if (FindFileFilterFn::filter(l_file_path))
					{
						out_paths_buffer.push_back(l_file_path);
					}
					else
					{
						l_file_path.free();
					}
				}

			}

			if (!FindNextFile(l_find, &l_find_data))
			{
				goto end;
			};
		}

	end:
		FindClose(l_find);
	};

};