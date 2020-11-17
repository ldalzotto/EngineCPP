
#include <string>
#include <sstream>
#include <fstream>
#include "../Render/Render/Render.cpp"
#include "Common/Functional/Hash.hpp"
#include "Common/Container/hashmap.hpp"

struct ObjReader
{
	struct VertexKey
	{
		size_t position_index;
		size_t uv_index;

		size_t computed_indice;

		inline bool equals(VertexKey& p_other)
		{
			return this->position_index == p_other.position_index && this->uv_index == p_other.uv_index;
		}

		/*
		inline size_t hash()
		{
			return HashFunctionRaw((char*)this, sizeof(VertexKey));
		};
		*/
	};


	inline static void ReadObj(const std::string& p_file_path, com::Vector<Vertex>& out_vertices, com::Vector<uint32_t>& out_indices)
	{
		std::ifstream l_file_stream(p_file_path);
		{
			com::Vector<VertexKey> l_vertices_indexed;
			// l_vertices_indexed.allocate(0);

			com::Vector<vec3f> positions;
			com::Vector<vec2f> uvs;
			positions.allocate(0);
			uvs.allocate(0);
			
			std::string l_line;
			while (std::getline(l_file_stream, l_line))
			{
				if (l_line.compare(0, strlen("vn"), "vn") == 0)
				{
				}
				else if (l_line.compare(0, strlen("vt"), "vt") == 0)
				{
					vec2f l_uv;

					size_t l_first_space = l_line.find(" ", strlen("vt"));
					size_t l_second_space = l_line.find(" ", l_first_space + 1);

					l_uv.x = (float)atof(l_line.substr(l_first_space + 1, l_second_space - l_first_space).c_str());
					l_uv.y = 1.0f - (float)atof(l_line.substr(l_second_space + 1, l_line.length() - l_second_space).c_str());

					uvs.push_back(l_uv);
				}
				else if (l_line.compare(0, strlen("v"), "v") == 0)
				{
					vec3f l_local_position;
					size_t l_first_space = l_line.find(" ", strlen("v"));
					size_t l_second_space = l_line.find(" ", l_first_space + 1);
					size_t l_third_space = l_line.find(" ", l_second_space + 1);

					std::string l_str = l_line.substr(l_first_space + 1, l_second_space - l_first_space);
					l_local_position.x = (float)atof(l_str.c_str());
					l_str = l_line.substr(l_second_space + 1, l_third_space - l_second_space);
					l_local_position.y = (float)atof(l_str.c_str());
					l_str = l_line.substr(l_third_space + 1, l_line.length() - l_third_space);
					l_local_position.z = (float)atof(l_str.c_str());

					positions.push_back(l_local_position);
				}

				else if (l_line.compare(0, strlen("f"), "f") == 0)
				{
					size_t l_first_space = l_line.find(" ", strlen("f"));
					size_t l_second_space = l_line.find(" ", l_first_space + 1);
					size_t l_third_space = l_line.find(" ", l_second_space + 1);
					size_t l_end = l_line.length();

					process_obj_face(l_line, l_first_space + 1, l_second_space, l_vertices_indexed, positions, uvs, out_vertices, out_indices);
					process_obj_face(l_line, l_second_space + 1, l_third_space, l_vertices_indexed, positions, uvs, out_vertices, out_indices);
					process_obj_face(l_line, l_third_space + 1, l_end, l_vertices_indexed, positions, uvs, out_vertices, out_indices);

					//l_line.substr(l_first_space, l_second_space - l_first_space).
				}
			}

			l_vertices_indexed.free();
			positions.free();
			uvs.free();
		}
		l_file_stream.close();
	}

	inline static void process_obj_face(const std::string& l_line, const size_t& l_first_space, const size_t& l_second_space, com::Vector<VertexKey>& l_vertices_indexed,
		com::Vector<vec3f>& positions, com::Vector<vec2f>& uvs, com::Vector<Vertex>& out_vertices, com::Vector<uint32_t>& out_indices)
	{
		std::string l_first_vertex = l_line.substr(l_first_space, l_second_space - l_first_space);
		size_t l_first_slash = l_first_vertex.find("/", 0);
		size_t l_second_slash = l_first_vertex.find("/", l_first_slash + 1);

		size_t l_position_index = atoi(l_first_vertex.substr(0, l_first_slash).c_str()) - 1;
		size_t l_uv_index = atoi(l_first_vertex.substr(l_first_slash + 1, l_second_slash - l_first_slash).c_str()) - 1;

		VertexKey l_key;
		l_key.position_index = l_position_index;
		l_key.uv_index = l_uv_index;

		bool l_key_already_indexed = false;
		size_t l_key_already_indexed_index = 0;
		for (size_t i = 0; i < l_vertices_indexed.Size; i++)
		{
			if (l_vertices_indexed[i].equals(l_key))
			{
				l_key_already_indexed = true;
				l_key.computed_indice = i;
				break;
			}
		}

		if (!l_key_already_indexed)
		{
			l_key.computed_indice = l_vertices_indexed.Size;
			l_vertices_indexed.push_back(l_key);
			Vertex l_vertex;
			l_vertex.position = positions[l_position_index];
			l_vertex.uv = uvs[l_uv_index];
			out_vertices.push_back(l_vertex);
		}

		out_indices.push_back((uint32_t)l_key.computed_indice);

	}
};