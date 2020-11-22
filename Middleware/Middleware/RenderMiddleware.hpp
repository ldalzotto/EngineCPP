#pragma once

#include "Common/Container/vector.hpp"
#include "SceneComponents/components.hpp"
#include "Scene/scene.hpp"
#include "Render/render.hpp"
#include <optick.h>

struct RenderableObjectEntry
{
	com::PoolToken node;
	RenderableObjectHandle renderableobject;

	RenderableObjectEntry() {}
};

struct RenderMiddleware
{
	RenderHandle render;
	AssetServerHandle asset_server;

	com::Vector<RenderableObjectEntry> allocated_renderableobjects;

	inline void allocate(RenderHandle p_render, AssetServerHandle p_asset_server)
	{
		this->render = p_render;
		this->asset_server = p_asset_server;
	};

	inline void free()
	{
		this->allocated_renderableobjects.free();
	};

	inline void on_elligible(const com::PoolToken p_node_token, const NTreeResolve<SceneNode>& p_node, const MeshRenderer& p_mesh_renderer)
	{
		com::Vector<char> l_material_binary = this->asset_server.get_resource(p_mesh_renderer.material);
		MaterialAsset l_material_asset = MaterialAsset::deserialize(l_material_binary.Memory); 
		l_material_binary.free();

		ShaderHandle l_shader;
		l_shader.allocate(this->render, l_material_asset.shader.vertex, l_material_asset.shader.fragment);
		TextureHandle l_texture;
		l_texture.allocate(this->render, l_material_asset.texture);
		MaterialHandle l_material;
		l_material.allocate(this->render, l_shader, l_texture);
		MeshHandle l_mesh;
		l_mesh.allocate(this->render, p_mesh_renderer.model);
		RenderableObjectHandle l_renderable_object;
		l_renderable_object.allocate(this->render, l_material, l_mesh);

		l_renderable_object.push_trs(this->render, p_node.element->get_localtoworld());

		RenderableObjectEntry l_entry;
		l_entry.node = p_node_token;
		l_entry.renderableobject = l_renderable_object;

		this->allocated_renderableobjects.push_back(l_entry);
	};

	inline void on_not_elligible(const com::PoolToken p_node_token)
	{
		for (size_t i = 0; i < this->allocated_renderableobjects.Size; i++)
		{
			RenderableObjectEntry& l_entry = this->allocated_renderableobjects[i];
			if (l_entry.node.Index == p_node_token.Index)
			{
				l_entry.renderableobject.free(this->render);
				this->allocated_renderableobjects.erase_at(i);
				return;
			}
		}
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