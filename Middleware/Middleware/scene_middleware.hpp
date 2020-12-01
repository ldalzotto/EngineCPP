#pragma once

#include "Scene/scene.hpp"
#include "RenderMiddleware.hpp"

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

