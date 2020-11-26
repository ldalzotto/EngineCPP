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
	l_render->heap.set_material(this->handle, this->material.handle, this->material.shader.handle, p_material.handle);
	this->material = p_material;
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
	this->handle = l_render->heap.allocate_shader(p_sahder_path).Index;
};

void ShaderHandle::allocate(const RenderHandle& p_render, const size_t p_shader_path)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_shader(p_shader_path).Index;
};

void ShaderHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_shader(this->handle);
	this->reset();
};

void MaterialHandle::allocate(const RenderHandle& p_render, const ShaderHandle& p_shader)
{
	Render* l_render = (Render*)p_render;
	this->shader = p_shader;
	this->handle = l_render->heap.allocate_material(this->shader.handle).Index;
};

void MaterialHandle::add_image_parameter(const RenderHandle& p_render, const TextureHandle& p_texture)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.material_add_image_parameter(this->handle, p_texture.handle);
};

void MaterialHandle::add_uniform_parameter(const RenderHandle& p_render, const GPtr& p_initial_value)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.material_add_uniform_parameter(this->handle, p_initial_value);
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