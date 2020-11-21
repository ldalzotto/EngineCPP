#include <interface/Engine/engine.hpp>
#include <Render/render.hpp>
#include <GLFW/glfwinclude.h>
#include <optick.h>
#include <Scene/scene.hpp>
#include <AssetServer/asset_server.hpp>
#include "engine_loop.hpp"

#include "SceneComponents/components.hpp"
#include "Middleware/RenderMiddleware.hpp"

struct Engine;

struct EngineCallbacks
{
	ExternalHooks external_hooks;

	Engine* closure;

	EngineCallbacks() {}

	inline EngineCallbacks(Engine* p_engine, const ExternalHooks& p_external_hooks)
	{
		this->closure = p_engine;
		this->external_hooks = p_external_hooks;
	};

	void newframe_callback();
	void update_callback(float p_delta);
	void endupdate_callback();
	void render_callback();
	void endofframe_callback();
};

struct Engine
{
	Clock clock;
	AssetServerHandle asset_server;
	EngineLoop<EngineCallbacks> loop;
	SceneHandle scene;
	RenderHandle render;

	RenderMiddleware render_middleware;

	Engine(const std::string& p_executeable_path, const ExternalHooks& p_hooks);
	void dispose();
	void mainloop();
};

struct SceneCallbacks
{
	static void on_component_added(Engine* p_engine, ComponentAddedParameter* p_parameter);
	static void on_component_removed(Engine* p_engine, ComponentRemovedParameter* p_parameter);
	static void push_componentasset(Engine* p_engine, ComponentAssetPushParameter* p_parameter);
};

inline Engine::Engine(const std::string& p_executeable_path, const ExternalHooks& p_hooks)
{
	this->asset_server.allocate(p_executeable_path);
	this->loop = EngineLoop<EngineCallbacks>(EngineCallbacks(this, p_hooks), 16000);
	this->scene.allocate(*(Callback<void, ComponentAddedParameter>*) & Callback<Engine, ComponentAddedParameter>(this, SceneCallbacks::on_component_added),
		*(Callback<void, ComponentRemovedParameter>*) & Callback<Engine, ComponentRemovedParameter>(this, SceneCallbacks::on_component_removed),
		*(Callback<void, ComponentAssetPushParameter>*) & Callback<Engine, ComponentAssetPushParameter>(this, SceneCallbacks::push_componentasset));
	this->render = create_render(this->asset_server);
	this->render_middleware.allocate(this->render);
}

inline void Engine::dispose()
{
	this->render_middleware.free();
	destroy_render(this->render);
	this->scene.free();
	this->asset_server.free();
}

inline void Engine::mainloop()
{
	while (!render_window_should_close(this->render))
	{
		render_window_pool_event(this->render);
		this->loop.update();
	}
}

EngineHandle engine_create(const std::string& p_executeable_path, const ExternalHooks& p_hooks)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	return new Engine(p_executeable_path, p_hooks);
};

void engine_mainloop(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->mainloop();
}


inline void EngineCallbacks::newframe_callback()
{
	OPTICK_FRAME("MainThread");
	this->closure->clock.newframe();
	this->closure->scene.new_frame();
}

inline void EngineCallbacks::update_callback(float p_delta)
{
	this->closure->clock.newupdate(p_delta);
	this->external_hooks.ext_update(this->external_hooks.closure, p_delta);
}

inline void EngineCallbacks::endupdate_callback()
{
}

inline void EngineCallbacks::render_callback()
{
	this->closure->render_middleware.pre_render(this->closure->scene);
	render_draw(this->closure->render);
}

inline void EngineCallbacks::endofframe_callback()
{
}


inline void SceneCallbacks::on_component_added(Engine* p_engine, ComponentAddedParameter* p_parameter)
{
	// if MeshRenderer, the push to render middleware
	switch (p_parameter->component->id)
	{
	case MeshRenderer::Id:
	{
		p_engine->render_middleware.on_elligible(p_parameter->node_token, p_parameter->node, *p_parameter->component->cast<MeshRenderer>());
	}
	break;
	}
};

inline void SceneCallbacks::on_component_removed(Engine* p_engine, ComponentRemovedParameter* p_paramter)
{
	switch (p_paramter->component->id)
	{
	case MeshRenderer::Id:
	{
		p_engine->render_middleware.on_not_elligible(p_paramter->node_token);
	}
	break;
	}
};

inline void SceneCallbacks::push_componentasset(Engine* p_engine, ComponentAssetPushParameter* p_parameter)
{
	switch (p_parameter->component_asset->id)
	{
	case MeshRenderer::Id:
	{
		MeshRendererAsset* l_asset = (MeshRendererAsset*)p_parameter->component_asset_object;
		
		com::Vector<char> l_material_asset_binary = p_engine->asset_server.get_resource(l_asset->material);
		MaterialAsset l_material_asset = MaterialAsset::deserialize(l_material_asset_binary.Memory);
		l_material_asset_binary.free();
		
		MeshRenderer l_mesh_renderer;
		l_mesh_renderer.initialize(l_material_asset, l_asset->mesh);
		p_engine->scene.add_component<MeshRenderer>(p_parameter->node, l_mesh_renderer);
	}
	break;
	}
};


void engine_destroy(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->dispose();
	delete ((Engine*)p_engine);
	glfwTerminate();
};

SceneHandle engine_scene(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->scene;
};

AssetServerHandle engine_assetserver(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->asset_server;
};