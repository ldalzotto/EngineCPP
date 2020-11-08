#include <interface/Engine/engine.hpp>
#include <Render/render.hpp>
#include <GLFW/glfwinclude.h>
#include <optick.h>
#include <Scene/scene.hpp>
#include "engine_loop.hpp"

struct Engine;

struct EngineCallbacks
{
	Engine* closure;

	EngineCallbacks(){}

	inline EngineCallbacks(Engine* p_engine)
	{
		this->closure = p_engine;
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
	EngineLoop<EngineCallbacks> loop;
	SceneHandle scene;
	RenderHandle render;
	Engine();
	void dispose();
	void mainloop();
};

struct SceneCallbacks
{
	static void on_component_added(Engine* p_engine, ComponentAddedParameter* p_parameter);
};

inline Engine::Engine()
{
	this->loop = EngineLoop<EngineCallbacks>(EngineCallbacks(this), 16000);
	this->scene.allocate(*(Callback<void, ComponentAddedParameter>*)&Callback<Engine, ComponentAddedParameter>(this, SceneCallbacks::on_component_added));
	this->render = create_render();
}

inline void Engine::dispose()
{
	destroy_render(this->render);
	this->scene.free();
}

inline void Engine::mainloop()
{
	while (!render_window_should_close(this->render))
	{
		render_window_pool_event(this->render);
		this->loop.update();
	}
}

EngineHandle engine_create()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	return new Engine();
};

void engine_mainloop(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->mainloop();
}


inline void EngineCallbacks::newframe_callback()
{
	OPTICK_FRAME("MainThread");
	this->closure->clock.newframe();
}

inline void EngineCallbacks::update_callback(float p_delta)
{
	this->closure->clock.newupdate(p_delta);
}

inline void EngineCallbacks::endupdate_callback()
{
}

inline void EngineCallbacks::render_callback()
{
	render_draw(this->closure->render);
}

inline void EngineCallbacks::endofframe_callback()
{
}

inline void SceneCallbacks::on_component_added(Engine* p_engine, ComponentAddedParameter* p_parameter)
{
	//TODO
	// if MeshRenderer, the push to render middleware
};

void engine_destroy(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->dispose();
	delete ((Engine*)p_engine);
	glfwTerminate();
};