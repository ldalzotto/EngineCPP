#pragma once

#include "Common/Memory/handle.hpp"
#include "Common/Container/vector_def.hpp"
#include "Math/matrix_def.hpp"
#include "Math/vector_def.hpp"

struct VertexResource
{
	Math::vec3f position;
	Math::vec3f color;
};

struct MeshHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const com::Vector<VertexResource>& p_vertices, const com::Vector<uint32_t>& p_indices);
	void free(const RenderHandle& p_render);
};

struct ShaderHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const std::string& p_vertex_shader, const std::string& p_fragment_shader);
	void free(const RenderHandle& p_render);
};

struct MaterialHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const ShaderHandle& p_shader);
	void free(const RenderHandle& p_render);
};

struct RenderableObjectHandle : public Handle
{
	void allocate(const RenderHandle& p_render, const MaterialHandle p_material, const MeshHandle p_meshhandle);
	void push_trs(const RenderHandle& p_render, const Math::mat4f& p_trs);
	void free(const RenderHandle& p_render);
};


