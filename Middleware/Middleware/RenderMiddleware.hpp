#pragma once

#include "Common/Container/pool.hpp"
#include "SceneComponents/components.hpp"
#include "Scene/scene.hpp"
#include "Scene/kernel/scene.hpp"
#include "Render/render.hpp"
#include <optick.h>

struct DefaultMaterial : public MaterialHandle
{
	inline static const size_t COLOR_INDEX = 1;

	inline void set_color(const RenderHandle& p_render, Math::vec4f& p_value)
	{
		this->set_uniform_parameter(p_render, DefaultMaterial::COLOR_INDEX, GPtr::fromType(&p_value));
	};

	inline Math::vec4f get_color(const RenderHandle& p_render)
	{
		Math::vec4f l_return;
		this->get_uniform_paramter(p_render, DefaultMaterial::COLOR_INDEX, GPtr::fromType(&l_return));
		return l_return;
	};
};

struct MaterialBuilder
{
	inline static DefaultMaterial build(MeshRenderer::MaterialKey p_material, AssetServerHandle& p_asset_server, RenderHandle& p_render)
	{
		com::Vector<char> l_material_binary = p_asset_server.get_resource(p_material.key);
		MaterialAsset l_material_asset = MaterialAsset::deserialize(l_material_binary.Memory);
		l_material_binary.free();

		ShaderHandle l_shader;
		l_shader.allocate(p_render, l_material_asset.shader);
		TextureHandle l_texture;
		l_texture.allocate(p_render, l_material_asset.texture);

		DefaultMaterial l_material;
		l_material.allocate(p_render, l_shader);
		l_material.add_image_parameter(p_render, l_texture);
		l_material.add_uniform_parameter(p_render, GPtr::fromType(&l_material_asset.color));

		return l_material;
	};
};

struct RenderableObjectEntry
{
	com::PoolToken node;
	RenderableObjectHandle renderableobject;
	DefaultMaterial default_material;

	// this boolean is used to push data to render when the MeshRenderer component is added after the node is created. Because in this case,
	// the haschanged_thisframe will be false.
	bool force_update = false;
	RenderableObjectEntry() {};

	inline void set_material(MeshRenderer* p_mesh_renderer, size_t p_new_material, AssetServerHandle& p_asset_server, RenderHandle& p_render)
	{
		this->default_material = MaterialBuilder::build(p_new_material, p_asset_server, p_render);
		this->renderableobject.set_material(p_render, this->default_material);
		p_mesh_renderer->material = p_new_material;
	};
};

struct CameraEntry
{
	com::PoolToken node;

	CameraEntry() {}
};

struct RenderMiddleware
{
	RenderHandle render;
	AssetServerHandle asset_server;

	CameraEntry allocated_camera;
	com::OptionalPool<RenderableObjectEntry> allocated_renderableobjects;

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

	inline RenderableObjectEntry* get_renderable_object(const com::TPoolToken<Optional<RenderableObjectEntry>>& p_renderable_object)
	{
		Optional<RenderableObjectEntry>& l_entry = this->allocated_renderableobjects[p_renderable_object];
		if (l_entry.hasValue)
		{
			return &l_entry.value;
		}
		return nullptr;
	};

	inline RenderableObjectEntry* get_renderable_object(MeshRenderer* p_mesh_renderer)
	{
		return this->get_renderable_object(com::TPoolToken<Optional<RenderableObjectEntry>>(p_mesh_renderer->rendererable_object.Index));
	};

	inline void on_elligible(const com::PoolToken p_node_token, const NTreeResolve<SceneNode>& p_node, MeshRenderer& p_mesh_renderer)
	{
		DefaultMaterial l_material = MaterialBuilder::build(p_mesh_renderer.material, this->asset_server, this->render);

		MeshHandle l_mesh;
		l_mesh.allocate(this->render, p_mesh_renderer.model.key);
		RenderableObjectHandle l_renderable_object;
		l_renderable_object.allocate(this->render, l_material, l_mesh);

		RenderableObjectEntry l_entry;
		l_entry.node = p_node_token;
		l_entry.renderableobject = l_renderable_object;
		l_entry.default_material = l_material;
		l_entry.force_update = true;

		com::PoolToken l_allocated_entry = this->allocated_renderableobjects.alloc_element(l_entry);
		p_mesh_renderer.rendererable_object = l_allocated_entry;
	};

	inline void on_not_elligible(const com::PoolToken p_node_token)
	{
		for (size_t i = 0; i < this->allocated_renderableobjects.size(); i++)
		{
			Optional<RenderableObjectEntry>& l_entry = this->allocated_renderableobjects[i];
			if (l_entry.hasValue)
			{
				if (l_entry.value.node.Index == p_node_token.Index)
				{
					l_entry.value.renderableobject.free(this->render);
					this->allocated_renderableobjects.release_element(i);
					return;
				}
			}

		}
	};

	inline void pre_render(Scene* p_scene)
	{
		OPTICK_EVENT();

		{
			if (this->allocated_camera.node.Index != -1)
			{
				NTreeResolve<SceneNode> l_camera_scene_node = SceneKernel::resolve_node(p_scene, this->allocated_camera.node);
				if (l_camera_scene_node.element->state.haschanged_thisframe)
				{
					Camera* l_camera = SceneKernel::get_component<Camera>(p_scene, this->allocated_camera.node);
					Math::mat4f& l_localtoworld = SceneKernel::get_localtoworld(l_camera_scene_node.element, p_scene);
					render_push_camera_buffer(this->render, l_camera->fov, l_camera->near_, l_camera->far_, SceneKernel::get_worldposition(l_camera_scene_node.element, p_scene), l_localtoworld.Forward.Vec3, l_localtoworld.Up.Vec3);
				}
			}
		}


		for (size_t i = 0; i < this->allocated_renderableobjects.size(); i++)
		{
			Optional<RenderableObjectEntry>& l_entry = this->allocated_renderableobjects[i];
			if (l_entry.hasValue)
			{
				NTreeResolve<SceneNode> l_scenenode = SceneKernel::resolve_node(p_scene, l_entry.value.node);
				if (l_scenenode.element->state.haschanged_thisframe || l_entry.value.force_update)
				{
					l_entry.value.renderableobject.push_trs(this->render, SceneKernel::get_localtoworld(l_scenenode.element, p_scene));
					l_entry.value.force_update = false;
				}
			}
		}
	};

	inline void set_material(MeshRenderer* p_mesh_renderer, size_t p_new_material)
	{
		this->get_renderable_object(com::TPoolToken<Optional<RenderableObjectEntry>>(p_mesh_renderer->rendererable_object.Index))->set_material(p_mesh_renderer, p_new_material, this->asset_server, this->render);
	};

	
};

typedef RenderMiddleware* RenderMiddlewareHandle;