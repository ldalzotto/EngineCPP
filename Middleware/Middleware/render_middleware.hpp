#pragma once

#include "Common/Container/pool.hpp"
#include "SceneComponents/components.hpp"
#include "Scene/scene.hpp"
#include "Scene/kernel/scene.hpp"
#include "Render/render.hpp"
#include <optick.h>

/*

The RenderMiddleware is the interface between the Render system and the Scene.
	- It captures MeshRenderer components and creates the associated Shader->Material->RenderableObject hierarchy.
	  Every frame, the middleware check if the scene node has moved. If so, the TRS matrix buffer is pushed to the render system.
	- It captures the Camera component and pushes it to global buffers.

*/

struct RenderableObjectEntry
{
	SceneNodeToken node;
	RenderableObjectHandle renderableobject;
	MaterialHandle material;

	// this boolean is used to push data to render when the MeshRenderer component is added after the node is created. Because in this case,
	// the haschanged_thisframe will be false.
	bool force_update = false;
	RenderableObjectEntry() {};

	inline void set_material(MeshRenderer& p_mesh_renderer, size_t p_new_material, AssetServerHandle& p_asset_server, RenderHandle& p_render)
	{
		this->material.allocate(p_render, p_new_material);
		this->renderableobject.set_material(p_render, this->material);
		p_mesh_renderer.material.key = p_new_material;
	};

	inline void set_mesh(MeshRenderer& p_mesh_renderer, size_t p_new_mesh, AssetServerHandle& p_asset_server, RenderHandle& p_render)
	{
		MeshHandle l_new_mesh;
		l_new_mesh.allocate(p_render, p_new_mesh);
		this->renderableobject.set_mesh(p_render, l_new_mesh);
		p_mesh_renderer.mesh.key = p_new_mesh;
	};
};

struct CameraEntry
{
	SceneNodeToken node;

	CameraEntry() {}
};

struct RenderMiddleware
{
	RenderHandle render;
	AssetServerHandle asset_server;

	CameraEntry allocated_camera;
	com::Pool<RenderableObjectEntry> allocated_renderableobjects;
	com::Vector<com::TPoolToken<RenderableObjectEntry>> allocated_renderableobjects_vector;

	inline void allocate(RenderHandle p_render, AssetServerHandle p_asset_server)
	{
		this->render = p_render;
		this->asset_server = p_asset_server;
	};

	inline void free()
	{
		this->allocated_renderableobjects.free_checked();
		this->allocated_renderableobjects_vector.free_checked();
	};

	inline void push_camera(const SceneNodeToken p_node_token, const NTreeResolve<SceneNode>& p_node, const Camera& p_camera)
	{
		this->allocated_camera.node = p_node_token;
	};

	inline void remove_camera()
	{
		this->allocated_camera.node = SceneNodeToken();
	};

	inline RenderableObjectEntry& get_renderable_object(const com::TPoolToken<RenderableObjectEntry>& p_renderable_object)
	{
#if RENDER_BOUND_TEST
		if(this->allocated_renderableobjects.is_token_free(p_renderable_object))
		{
			abort();
		}
#endif
		return this->allocated_renderableobjects[p_renderable_object];
	};

	inline RenderableObjectEntry& get_renderable_object(const MeshRenderer& p_mesh_renderer)
	{
		return this->get_renderable_object(com::TPoolToken<RenderableObjectEntry>(p_mesh_renderer.rendererable_object.val));
	};

	inline void on_elligible(const SceneNodeToken p_node_token, const NTreeResolve<SceneNode>& p_node, MeshRenderer& p_mesh_renderer)
	{
		MaterialHandle l_material;
		l_material.allocate(this->render, p_mesh_renderer.material.key);

		MeshHandle l_mesh;
		l_mesh.allocate(this->render, p_mesh_renderer.mesh.key);

		RenderableObjectHandle l_renderable_object;
		l_renderable_object.allocate(this->render, l_material, l_mesh);

		RenderableObjectEntry l_entry;
		l_entry.node = p_node_token;
		l_entry.renderableobject = l_renderable_object;
		l_entry.material = l_material;
		l_entry.force_update = true;

		com::TPoolToken<RenderableObjectEntry> l_allocated_entry = this->allocated_renderableobjects.alloc_element(l_entry);
		this->allocated_renderableobjects_vector.push_back(com::TPoolToken<RenderableObjectEntry>(l_allocated_entry.val));
		p_mesh_renderer.rendererable_object = l_allocated_entry.to_pooltoken();
	};

	inline void on_not_elligible(const com::PoolToken p_node_token)
	{
		for (size_t i = 0; i < this->allocated_renderableobjects_vector.Size; i++)
		{
			com::TPoolToken<RenderableObjectEntry>& l_entry_token = this->allocated_renderableobjects_vector[i];
			RenderableObjectEntry& l_entry = this->allocated_renderableobjects[l_entry_token];
			if (l_entry.node.val == p_node_token.val)
			{
				l_entry.renderableobject.free(this->render);
				this->allocated_renderableobjects.release_element(l_entry_token);
				this->allocated_renderableobjects_vector.erase_at(i, 1);
				break;
			}
		}
	};

	inline void pre_render(Scene* p_scene)
	{
		OPTICK_EVENT();

		{
			if (this->allocated_camera.node.val != -1)
			{
				NTreeResolve<SceneNode> l_camera_scene_node = SceneKernel::resolve_node(p_scene, this->allocated_camera.node);
				if (l_camera_scene_node.element->state.haschanged_thisframe)
				{
					Camera& l_camera = SceneKernel::get_component<Camera>(p_scene, this->allocated_camera.node);
					Math::mat4f& l_localtoworld = SceneKernel::get_localtoworld(l_camera_scene_node.element, p_scene);
					render_push_camera_buffer(this->render, l_camera.fov, l_camera.near_, l_camera.far_, 
						SceneKernel::get_worldposition(l_camera_scene_node.element, p_scene), l_localtoworld.Forward.Vec3, l_localtoworld.Up.Vec3);
				}
			}
		}

		for (size_t i = 0; i < this->allocated_renderableobjects_vector.Size; i++)
		{
			RenderableObjectEntry& l_entry = this->allocated_renderableobjects[this->allocated_renderableobjects_vector[i]];
			NTreeResolve<SceneNode> l_scenenode = SceneKernel::resolve_node(p_scene, l_entry.node);
			if (l_scenenode.element->state.haschanged_thisframe || l_entry.force_update)
			{
				l_entry.renderableobject.push_trs(this->render, SceneKernel::get_localtoworld(l_scenenode.element, p_scene));
				l_entry.force_update = false;
			}
		}
	};

	inline void set_material(MeshRenderer& p_mesh_renderer, size_t p_new_material)
	{
		this->get_renderable_object(com::TPoolToken<RenderableObjectEntry>(p_mesh_renderer.rendererable_object.val)).set_material(p_mesh_renderer, p_new_material, this->asset_server, this->render);
	};

	inline void set_mesh(MeshRenderer& p_mesh_renderer, size_t p_new_mesh)
	{
		this->get_renderable_object(com::TPoolToken<RenderableObjectEntry>(p_mesh_renderer.rendererable_object.val)).set_mesh(p_mesh_renderer, p_new_mesh, this->asset_server, this->render);
	};

};

typedef RenderMiddleware* RenderMiddlewareHandle;