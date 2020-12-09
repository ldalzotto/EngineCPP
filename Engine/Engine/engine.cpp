#include <interface/Engine/engine.hpp>
#include <Render/render.hpp>
#include <GLFW/glfwinclude.h>
#include <optick.h>
#include <Scene/scene.hpp>
#include <Scene/kernel/scene.hpp>
#include <AssetServer/asset_server.hpp>
#include "engine_loop.hpp"
#include "Input/input.hpp"

#include "SceneComponents/components.hpp"
#include "Middleware/render_middleware.hpp"
#include "Middleware/scene_middleware.hpp"

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
	Scene scene;
	RenderHandle render;
	InputHandle input;

	RenderMiddleware render_middleware;
	ComponentMiddlewares all_middlewares;

	Engine(const std::string& p_executeable_path, const ExternalHooks& p_hooks);
	void dispose();
	bool should_close();
	void mainloop();
	void poll_events();
	void singleframe();
	void singleframe(float p_delta);
};

inline Engine::Engine(const std::string& p_executeable_path, const ExternalHooks& p_hooks)
{
	this->asset_server.allocate(p_executeable_path);
	this->loop = EngineLoop<EngineCallbacks>(EngineCallbacks(this, p_hooks), 16000);
	SceneKernel::allocate_scene(&this->scene, *(Callback<void, ComponentAddedParameter>*) & Callback<ComponentMiddlewares, ComponentAddedParameter>(&this->all_middlewares, SceneComponentCallbacks::on_component_added),
		*(Callback<void, ComponentRemovedParameter>*) & Callback<ComponentMiddlewares, ComponentRemovedParameter>(&this->all_middlewares, SceneComponentCallbacks::on_component_removed),
		*(Callback<void, ComponentAssetPushParameter>*) & Callback<void, ComponentAssetPushParameter>(nullptr, SceneComponentCallbacks::push_componentasset2));
	this->render = create_render(this->asset_server);
	this->input.allocate(render_window(this->render));
	this->render_middleware.allocate(this->render, this->asset_server);
	this->all_middlewares.render_middleware = &this->render_middleware;
}

inline void Engine::dispose()
{
	SceneKernel::free_scene(&this->scene);
	this->render_middleware.free();
	this->input.free();
	destroy_render(this->render);
	this->asset_server.free();
}

inline void Engine::mainloop()
{
	while (!this->should_close())
	{
		this->singleframe();
	}
}

inline bool Engine::should_close()
{
	return render_window_should_close(this->render);
}

inline void Engine::poll_events()
{
	render_window_pool_event(this->render);
}

inline void Engine::singleframe()
{
	render_window_pool_event(this->render);
	this->loop.update();
}

inline void Engine::singleframe(float p_delta)
{
	render_window_pool_event(this->render);
	this->loop.update_forced_delta(p_delta);
};

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

bool engine_should_close(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->should_close();
};

void engine_poll_events(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->poll_events();
};

void engine_singleframe(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->singleframe();
};

void engine_singleframe(const EngineHandle& p_engine, float p_delta)
{
	((Engine*)p_engine)->singleframe(p_delta);
};

inline void EngineCallbacks::newframe_callback()
{
	OPTICK_FRAME("MainThread");
	this->closure->clock.newframe();
	this->closure->input.new_frame();
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
	this->closure->render_middleware.pre_render(&this->closure->scene);
	render_draw(this->closure->render);
}

inline void EngineCallbacks::endofframe_callback()
{
	this->closure->scene.end_of_frame();
}




void engine_destroy(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->dispose();
	delete ((Engine*)p_engine);
};

void engine_exit(const EngineHandle& p_engine)
{
	engine_destroy(p_engine);
	glfwTerminate();
}

Scene* engine_scene(const EngineHandle& p_engine)
{
	return &((Engine*)p_engine)->scene;
};

AssetServerHandle engine_assetserver(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->asset_server;
};

WindowHandle engine_window(const EngineHandle& p_engine)
{
	return render_window(((Engine*)p_engine)->render);
};

InputHandle engine_input(const EngineHandle& p_engine)
{
	return ((Engine*)p_engine)->input;
};

RenderMiddlewareHandle engine_render_middleware(const EngineHandle& p_engine)
{
	return &((Engine*)p_engine)->render_middleware;
};


Clock* engine_clock(const EngineHandle& p_engine)
{
	return &((Engine*)p_engine)->clock;
};