#pragma once

#include <stdint.h>
#include <string>
#include <AssetServer/asset_server.hpp>
#include "Common/Memory/handle.hpp"
#include "Common/Container/vector_def.hpp"
#include "Math/matrix_def.hpp"
#include "Math/vector_def.hpp"

typedef void* RenderHandle;

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



RenderHandle create_render(const AssetServerHandle p_assetserver_handle);
void destroy_render(const RenderHandle& p_render);
bool render_window_should_close(const RenderHandle& p_render);
void render_window_pool_event(const RenderHandle& p_render);
void render_draw(const RenderHandle& p_render);

void render_allocate_renderableobject(const RenderHandle& p_render, const std::string& p_vertex_shader, const std::string& p_fragment_shader, const std::string& p_mesh, const Math::mat4f& p_initial_trs,
	MeshHandle& out_mesh, ShaderHandle& out_shader, MaterialHandle& out_material, RenderableObjectHandle& out_renderableobject);