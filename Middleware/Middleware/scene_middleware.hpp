#pragma once

#include "Scene/scene.hpp"
#include "SceneSerialization/scene_serialization.hpp"
#include "render_middleware.hpp"

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
		switch (p_parameter->component->type->id)
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
		switch (p_paramter->component->type->id)
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

	inline static void push_componentasset2(void* p_null, ComponentAssetPushParameter* p_parameter)
	{
		struct WithComponent
		{
			ComponentAssetPushParameter* parameter;

			inline WithComponent(ComponentAssetPushParameter* p_parameter)
			{
				this->parameter = p_parameter;
			};

			inline void with_component(void* p_component_object, const SceneNodeComponent_TypeInfo& p_type)
			{
				this->parameter->inserted_component = SceneKernel::add_component((Scene*)this->parameter->scene, this->parameter->node, p_type, p_component_object);
			}
		};

		ComponentAssetSerializer::ComponentAsset_to_Component(p_parameter->component_asset->id, p_parameter->component_asset_object, WithComponent(p_parameter));
	};
};

