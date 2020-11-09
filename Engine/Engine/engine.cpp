#include <interface/Engine/engine.hpp>
#include <Render/render.hpp>
#include <GLFW/glfwinclude.h>
#include <optick.h>
#include <Scene/scene.hpp>
#include "engine_loop.hpp"

#include "SceneComponents/components.hpp"
#include "Middleware/RenderMiddleware.hpp"

struct Engine;

struct EngineCallbacks
{
	ExternalHooks external_hooks;

	Engine* closure;

	EngineCallbacks(){}

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
	EngineLoop<EngineCallbacks> loop;
	SceneHandle scene;
	RenderHandle render;

	RenderMiddleware render_middleware;

	Engine(const ExternalHooks& p_hooks);
	void dispose();
	void mainloop();
};

struct SceneCallbacks
{
	static void on_component_added(Engine* p_engine, ComponentAddedParameter* p_parameter);
};

inline Engine::Engine(const ExternalHooks& p_hooks)
{
	this->loop = EngineLoop<EngineCallbacks>(EngineCallbacks(this, p_hooks), 16000);
	this->scene.allocate(*(Callback<void, ComponentAddedParameter>*)&Callback<Engine, ComponentAddedParameter>(this, SceneCallbacks::on_component_added));
	this->render = create_render();
	this->render_middleware.allocate(this->render);
}

inline void Engine::dispose()
{
	this->render_middleware.free();
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

EngineHandle engine_create(const ExternalHooks& p_hooks)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	return new Engine(p_hooks);
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
	this->external_hooks.ext_update(this->external_hooks.closure, p_delta);
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

	/*
	struct ComponentPresence
	{
		size_t index = -1;
		bool value = false;

		inline void set_present()
		{

		};
	};
	*/

	switch (p_parameter->component->id)
	{
	case MeshRenderer::Id:
	{
		p_engine->render_middleware.on_elligible(p_parameter->node_token, *p_parameter->component->cast<MeshRenderer>());
		//TODO -> insert to middleware
		/*
		struct ComponentsPresence
		{
			ComponentPresence meshrenderer = ComponentPresence();
		};

		ComponentsPresence l_presence = ComponentsPresence();
		const auto& l_components = p_parameter->node.element->get_components();
		for (size_t i = 0; i < l_components.Size; i++)
		{
			SceneNodeComponentHeader* l_header = p_engine->scene.resolve_componentheader(l_components.Memory[i]);
			
		}
		*/
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