#pragma once

#include "Common/Container/vector.hpp"
#include "SceneComponents/components.hpp"
#include "Scene/scene.hpp"
#include "Render/render.hpp"

struct RenderableObjectEntry
{
	com::PoolToken<SceneNode> node;
	MeshHandle mesh;
	ShaderHandle shader;
	MaterialHandle material;
	RenderableObjectHandle renderableobject;

	RenderableObjectEntry() {}
};

struct RenderMiddleware
{
	RenderHandle render;

	com::Vector<RenderableObjectEntry> allocated_renderableobjects;

	inline void allocate(RenderHandle p_render)
	{
		this->render = p_render;
	};

	inline void free()
	{
		this->allocated_renderableobjects.free();
	};

	inline void on_elligible(const com::PoolToken<SceneNode>& p_node, const MeshRenderer& p_mesh_renderer)
	{
		RenderableObjectEntry l_entry;
		l_entry.node = p_node;
		render_allocate_renderableobject(this->render, p_mesh_renderer.vertex_shader, p_mesh_renderer.fragment_shader, p_mesh_renderer.model, l_entry.mesh, l_entry.shader, l_entry.material, l_entry.renderableobject);
		this->allocated_renderableobjects.push_back(l_entry);
	};

	inline void pre_render(SceneHandle p_scene)
	{
		/*
		for (size_t i = 0; i < this->allocated_renderableobjects.Size; i++)
		{
			p_scene.resolve_node(this->allocated_renderableobjects[i].node).element.;
		}
		*/
	};
};