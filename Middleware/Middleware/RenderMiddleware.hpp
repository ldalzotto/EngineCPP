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

struct CameraEntry
{
	com::PoolToken node;

	CameraEntry(){}
};

struct RenderMiddleware
{
	RenderHandle render;
	AssetServerHandle asset_server;

	CameraEntry allocated_camera;
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

	inline void push_camera(const com::PoolToken p_node_token, const NTreeResolve<SceneNode>& p_node, const Camera& p_camera)
	{
		this->allocated_camera.node = p_node_token;
	};

	inline void remove_camera()
	{
		this->allocated_camera.node = com::PoolToken();
	};

	inline void on_elligible(const com::PoolToken p_node_token, const NTreeResolve<SceneNode>& p_node, const MeshRenderer& p_mesh_renderer)
	{
		com::Vector<char> l_material_binary = this->asset_server.get_resource(p_mesh_renderer.material);
		MaterialAsset l_material_asset = MaterialAsset::deserialize(l_material_binary.Memory); 
		l_material_binary.free();

		ShaderHandle l_shader;
		l_shader.allocate(this->render, l_material_asset.shader);
		TextureHandle l_texture;
		l_texture.allocate(this->render, l_material_asset.texture);

		MaterialHandle l_material;
		l_material.allocate(this->render, l_shader);
		l_material.add_image_parameter(this->render, l_texture);
		l_material.add_uniform_parameter(this->render, GPtr::fromType(&l_material_asset.color));

		MeshHandle l_mesh;
		l_mesh.allocate(this->render, p_mesh_renderer.model);
		RenderableObjectHandle l_renderable_object;
		l_renderable_object.allocate(this->render, l_material, l_mesh);

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

		{
			if (this->allocated_camera.node.Index != -1)
			{
				NTreeResolve<SceneNode> l_camera_scene_node = p_scene.resolve_node(this->allocated_camera.node);
				if (l_camera_scene_node.element->state.haschanged_thisframe)
				{
					Camera* l_camera = p_scene.get_component<Camera>(this->allocated_camera.node);
					Math::mat4f& l_localtoworld = l_camera_scene_node.element->get_localtoworld();
					render_push_camera_buffer(this->render, l_camera->fov, l_camera->near_, l_camera->far_, l_camera_scene_node.element->get_worldposition(), l_localtoworld.Forward.Vec3, l_localtoworld.Up.Vec3);
				}
			}
		}
		

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