#include <interface/Engine/engine.hpp>
#include <Render/render.hpp>
#include <GLFW/glfwinclude.h>

struct Engine
{
	RenderHandle render;
	Engine();
	void dispose();
	void mainloop();
};

inline Engine::Engine()
{
	this->render = create_render();
}

inline void Engine::dispose()
{
	destroy_render(this->render);
}

inline void Engine::mainloop()
{
	while (!render_window_should_close(this->render))
	{

		render_window_pool_event(this->render);
	}
}

EngineHandle engine_create()
{
	glfwInit();
	return new Engine();
};

void engine_mainloop(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->mainloop();
}

void engine_destroy(const EngineHandle& p_engine)
{
	((Engine*)p_engine)->dispose();
	delete ((Engine*)p_engine);
	glfwTerminate();
};


