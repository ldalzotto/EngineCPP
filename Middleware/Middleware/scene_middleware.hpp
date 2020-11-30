#pragma once

#include "Scene/assets.hpp"
#include "Scene/scene.hpp"
#include "RenderMiddleware.hpp"
#include "SceneSerialization/scene_serialization.hpp"
#include "AssetServer/asset_server.hpp"

struct ComponentAssetSerializer
{
	inline static bool deserializeJSON(StringSlice& p_component_type, Deserialization::JSON::JSONObjectIterator& p_component_object_iterator,
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
		else if (p_component_type.equals("Camera"))
		{
			out_component_asset->id = Camera::Id;
			allocate_component_asset<CameraAsset>(p_compoent_asset_heap, &out_component_asset->componentasset_heap_index);

			CameraAsset* l_camera_asset = p_compoent_asset_heap.map<CameraAsset>(out_component_asset->componentasset_heap_index);
			*l_camera_asset = JSONDeserializer<CameraAsset>::deserialize(p_component_object_iterator);

			return true;
		}

		return false;
	};

	inline static void serializeJSON(ComponentAsset& p_component_asset, Serialization::JSON::Deserializer& p_serializer, GeneralPurposeHeap<>& p_compoent_asset_heap, AssetServerHandle p_asset_server)
	{
		StringSlice l_component_type;
		bool l_component_detected = false;
		if (p_component_asset.id == MeshRenderer::Id)
		{
			l_component_type = "MeshRenderer";
			l_component_detected = true;
		}
		else if (p_component_asset.id == Camera::Id)
		{
			l_component_type = "Camera";
			l_component_detected = true;
		}

		if (l_component_detected)
		{
			void* l_component = p_compoent_asset_heap.map<void>(p_component_asset.componentasset_heap_index);

			p_serializer.push_field("type", l_component_type);
			p_serializer.start_object("object");

			switch (p_component_asset.id)
			{
			case MeshRenderer::Id:
			{
				JSONSerializer<MeshRendererAsset>::serialize(p_serializer, *(MeshRendererAsset*)l_component, p_asset_server);
			}
			break;
			case Camera::Id:
			{
				JSONSerializer<CameraAsset>::serialize(p_serializer, *(CameraAsset*)l_component);
			}
			break;
			}

			p_serializer.end_object();
		}
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
		case Camera::Id:
		{
			p_component_middlewares->render_middleware->push_camera(p_parameter->node_token, p_parameter->node, *p_parameter->component->cast<Camera>());
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
		case Camera::Id:
		{
			p_component_middlewares->render_middleware->remove_camera();
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
			p_parameter->inserted_component = SceneKernel::add_component<MeshRenderer>((Scene*)p_parameter->scene, p_parameter->node, l_mesh_renderer);
		}
		break;
		case Camera::Id:
		{
			CameraAsset* l_asset = (CameraAsset*)p_parameter->component_asset_object;
			p_parameter->inserted_component = SceneKernel::add_component<Camera>((Scene*)p_parameter->scene, p_parameter->node, Camera(*l_asset));
		}
		break;
		}

	};
};

