#pragma once

#include "Scene/serialization.hpp"
#include "Scene/scene.hpp"
#include "RenderMiddleware.hpp"

struct ComponentAssetSerializer
{
	inline static bool deserializeJSON(StringSlice& p_component_type, Serialization::JSON::JSONObjectIterator& p_component_object_iterator,
		com::Vector<ComponentAsset>& p_component_assets, GeneralPurposeHeap<>& p_compoent_asset_heap, ComponentAsset* out_component_asset)
	{
		if (p_component_type.equals("MeshRenderer"))
		{
			out_component_asset->id = MeshRenderer::Id;

			allocate_component_asset<MeshRendererAsset>(p_compoent_asset_heap, &out_component_asset->componentasset_heap_index);

			MeshRendererAsset* l_mesh_renderer_asset = p_compoent_asset_heap.map<MeshRendererAsset>(out_component_asset->componentasset_heap_index);
			*l_mesh_renderer_asset = JSONDeserializer<MeshRendererAsset>::deserialize(p_component_object_iterator);

			return true;
		}

		return false;
	};

private:
	template<class ComponentAssetType>
	inline static bool allocate_component_asset(GeneralPurposeHeap<>& p_compoent_asset_heap, com::PoolToken* out_index)
	{
		while (!p_compoent_asset_heap.allocate_element(sizeof(ComponentAssetType), out_index))
		{
			p_compoent_asset_heap.realloc(p_compoent_asset_heap.memory.Capacity == 0 ? 100 : p_compoent_asset_heap.memory.Capacity * 2);
		};
		return true;
	};
};

struct ComponentMiddlewares
{
	RenderMiddleware* render_middleware = nullptr;

	inline ComponentMiddlewares() {};
	inline ComponentMiddlewares(RenderMiddleware* p_render_middleware) {
		this->render_middleware = p_render_middleware;
	}
};

struct SceneComponentCallbacks
{

	inline static void on_component_added(ComponentMiddlewares* p_component_middlewares, ComponentAddedParameter* p_parameter)
	{
		// if MeshRenderer, the push to render middleware
		switch (p_parameter->component->id)
		{
		case MeshRenderer::Id:
		{
			p_component_middlewares->render_middleware->on_elligible(p_parameter->node_token, p_parameter->node, *p_parameter->component->cast<MeshRenderer>());
		}
		break;
		}
	};

	inline static void on_component_removed(ComponentMiddlewares* p_component_middlewares, ComponentRemovedParameter* p_paramter)
	{
		switch (p_paramter->component->id)
		{
		case MeshRenderer::Id:
		{
			p_component_middlewares->render_middleware->on_not_elligible(p_paramter->node_token);
		}
		break;
		}
	};

	inline static void push_componentasset(void* p_null, ComponentAssetPushParameter* p_parameter)
	{
		switch (p_parameter->component_asset->id)
		{
		case MeshRenderer::Id:
		{
			MeshRendererAsset* l_asset = (MeshRendererAsset*)p_parameter->component_asset_object;

			MeshRenderer l_mesh_renderer;
			l_mesh_renderer.initialize(l_asset->material, l_asset->mesh);
			SceneHandle l_scene_handle;
			l_scene_handle.handle = p_parameter->scene;
			l_scene_handle.add_component<MeshRenderer>(p_parameter->node, l_mesh_renderer);
		}
		break;
		}
	};
};

