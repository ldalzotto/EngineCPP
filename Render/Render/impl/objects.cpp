#pragma once

#include "Render/render.hpp"

void RenderableObjectHandle::allocate(const RenderHandle& p_render, const MaterialHandle p_material, const MeshHandle p_meshhandle)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocateRendereableObject(com::PoolToken<Optional<Material>>(p_material.handle), com::PoolToken<Mesh>(p_meshhandle.handle), l_render->renderApi).Index;
};

void RenderableObjectHandle::push_trs(const RenderHandle& p_render, const Math::mat4f& p_trs)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.renderableobjects[this->handle].value.pushModelMatrix(p_trs, l_render->renderApi.device);
};

void RenderableObjectHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.freeRenderableObject(com::PoolToken<Optional<RenderableObject>>(this->handle), l_render->renderApi.device, l_render->renderApi.descriptor_pool);
	this->reset();
};

void ShaderHandle::allocate(const RenderHandle& p_render, const std::string& p_vertex_shader, const std::string& p_fragment_shader)
{
	Render* l_render = (Render*)p_render;
	ShaderResourceKey l_key;
	l_key.fragment_module = Hash<std::string>::hash(p_vertex_shader);
	l_key.fragment_module = Hash<std::string>::hash(p_fragment_shader);
	this->handle = l_render->resource_loader.shaders.allocate_resource(l_key).Index;
		//->heap.allocateShader(p_vertex_shader, p_fragment_shader, l_render->resource_loader, l_render->renderApi.swap_chain.renderpass, l_render->renderApi).Index;
};

void ShaderHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.freeShader(com::PoolToken<Optional<Shader>>(this->handle), l_render->renderApi.device, l_render->renderApi.descriptor_pool);
	this->reset();
};

void MaterialHandle::allocate(const RenderHandle& p_render, const ShaderHandle& p_shader)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocateMaterial(com::PoolToken<Optional<Shader>>(p_shader.handle)).Index;
};

void MaterialHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.freeMaterial(com::PoolToken<Optional<Material>>(this->handle), l_render->renderApi.device, l_render->renderApi.descriptor_pool);
	this->reset();
};

void MeshHandle::allocate(const RenderHandle& p_render, const com::Vector<VertexResource>& p_vertices, const com::Vector<uint32_t>& p_indices)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocateMesh(*(const com::Vector<Vertex>*) & p_vertices, p_indices, l_render->renderApi).Index;
};

void MeshHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.disposeMesh(com::PoolToken<Mesh>(this->handle), l_render->renderApi);
	this->reset();
}

