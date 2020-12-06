#pragma once

#include "Render/render.hpp"

void RenderableObjectHandle::allocate(const RenderHandle& p_render, const MaterialHandle p_material, const MeshHandle p_meshhandle)
{
	Render* l_render = (Render*)p_render;
	this->material = p_material;
	this->mesh = p_meshhandle;
	this->handle = l_render->heap.allocate_rendereableObject(this->material.handle, this->mesh.handle).Index;
};

void RenderableObjectHandle::push_trs(const RenderHandle& p_render, const Math::mat4f& p_trs)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.renderableobjects[this->handle].pushModelMatrix(p_trs, l_render->renderApi.device);
};

void RenderableObjectHandle::set_material(const RenderHandle& p_render, const MaterialHandle& p_material)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.set_material(com::TPoolToken<RenderableObject>(this->handle),
		com::TPoolToken<Material>(this->material.handle), com::TPoolToken<Shader>(this->material.shader.handle), com::TPoolToken<Material>(p_material.handle));
	this->material.free(p_render);
	this->material = p_material;
};

void RenderableObjectHandle::set_mesh(const RenderHandle& p_render, const MeshHandle& p_mesh)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.set_mesh(com::TPoolToken<RenderableObject>(this->handle), com::TPoolToken<Mesh>(p_mesh.handle));
	this->mesh.free(p_render);
	this->mesh = p_mesh;
};

void RenderableObjectHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	this->mesh.free(p_render);
	this->material.free(p_render);
	l_render->heap.free_renderableObject(this->handle);
	this->reset();
};

void ShaderHandle::allocate(const RenderHandle& p_render, const std::string& p_sahder_path)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_shader(p_sahder_path, l_render->renderApi.swap_chain.render_passes.get_renderpass<RenderPass::Type::RT_COLOR_DEPTH>()).Index;
};

void ShaderHandle::allocate(const RenderHandle& p_render, const size_t p_shader_path)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_shader(p_shader_path, l_render->renderApi.swap_chain.render_passes.get_renderpass<RenderPass::Type::RT_COLOR_DEPTH>()).Index;
};

void ShaderHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_shader(this->handle);
	this->reset();
};

void MaterialHandle::allocate(const RenderHandle& p_render, const size_t p_material)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_material(p_material, (com::PoolToken*)&this->shader.handle).Index;
};

void MaterialHandle::set_uniform_parameter(const RenderHandle& p_render, const size_t p_parameter_index, const GPtr& p_value)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.material_set_uniform_paramter(this->handle, p_parameter_index, p_value);
};

void MaterialHandle::get_uniform_paramter(const RenderHandle& p_render, size_t p_parameter_index, GPtr& out_value)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.material_get_uniform_paramter(this->handle, p_parameter_index, out_value);
};


void MaterialHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_material(this->handle, this->shader.handle);
	this->shader.free(p_render);
	this->reset();
};

void MeshHandle::allocate(const RenderHandle& p_render, const std::string& p_mesh)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_mesh(p_mesh).Index;
};

void MeshHandle::allocate(const RenderHandle& p_render, const size_t p_mesh)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_mesh(p_mesh).Index;
};

void MeshHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_mesh(this->handle);
	this->reset();
}

void TextureHandle::allocate(const RenderHandle& p_render, const std::string& p_texture)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_texture(p_texture).Index;
};

void TextureHandle::allocate(const RenderHandle& p_render, const size_t p_texture)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_texture(p_texture).Index;
};

void TextureHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_texture(this->handle);
	this->reset();
};