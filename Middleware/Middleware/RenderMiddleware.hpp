#pragma once

#include "Common/Container/vector.hpp"
#include "SceneComponents/components.hpp"
#include "Scene/scene.hpp"
#include "Render/render.hpp"
#include <optick.h>

struct RenderableObjectEntry
{
	com::PoolToken node;
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

	inline void on_elligible(const com::PoolToken p_node_token, const NTreeResolve<SceneNode>& p_node, const MeshRenderer& p_mesh_renderer)
	{
		RenderableObjectEntry l_entry;
		l_entry.node = p_node_token;
		render_allocate_renderableobject(this->render, p_mesh_renderer.vertex_shader, p_mesh_renderer.fragment_shader, p_mesh_renderer.model,
			"textures/16.09_diffuse.jpg", l_entry.mesh, l_entry.shader, l_entry.material, l_entry.renderableobject);
		
		l_entry.renderableobject.push_trs(this->render, p_node.element->get_localtoworld());

		this->allocated_renderableobjects.push_back(l_entry);
	};

	inline void pre_render(SceneHandle p_scene)
	{
		OPTICK_EVENT();

		for (size_t i = 0; i < this->allocated_renderableobjects.Size; i++)
		{
			RenderableObjectEntry& l_entry = this->allocated_renderableobjects[i];
			NTreeResolve<SceneNode> l_scenenode = p_scene.resolve_node(l_entry.node);
			if (l_scenenode.element->state.haschanged_thisframe)
			{
				l_entry.renderableobject.push_trs(this->render, l_scenenode.element->get_localtoworld());
			}
		}
	};
};