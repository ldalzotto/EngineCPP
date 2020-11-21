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
	l_render->heap.renderableobjects[this->handle].value.pushModelMatrix(p_trs, l_render->renderApi.device);
};

void RenderableObjectHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	this->mesh.free(p_render);
	l_render->heap.free_renderableObject(this->handle);
	this->material.free(p_render);
	this->reset();
};

void ShaderHandle::allocate(const RenderHandle& p_render, const std::string& p_vertex_shader, const std::string& p_fragment_shader)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_shader(p_vertex_shader, p_fragment_shader).Index;
};

void ShaderHandle::allocate(const RenderHandle& p_render, const size_t p_vertex_shader, const size_t p_fragment_shader)
{
	Render* l_render = (Render*)p_render;
	this->handle = l_render->heap.allocate_shader(p_vertex_shader, p_fragment_shader).Index;
};

void ShaderHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_shader(this->handle);
	this->reset();
};

void MaterialHandle::allocate(const RenderHandle& p_render, const ShaderHandle& p_shader, const TextureHandle& p_texture)
{
	Render* l_render = (Render*)p_render;
	this->shader = p_shader;
	this->texture = p_texture;
	this->handle = l_render->heap.allocate_material(this->shader.handle, this->texture.handle).Index;
};

void MaterialHandle::free(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	l_render->heap.free_material(this->handle);
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