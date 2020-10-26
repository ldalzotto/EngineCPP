#include <interface/Render/render.hpp>
#include <driver/Render/rdwindow.hpp>

struct Window
{
	WindowHandle Handle;
	Window() = default;
	Window(const short int p_width, const short int p_height, const std::string& p_title);
	void dispose();
};

struct Render
{
	Window window;
	Render();
	void dispose();
};

inline Window::Window(const short int p_width, const short int p_height, const std::string& p_title)
{
	this->Handle = rdwindow::create_window(p_width, p_height, p_title);
}

inline void Window::dispose()
{
	rdwindow::free_window(this->Handle);
}

inline Render::Render()
{
	this->window = Window(800, 600, "MyGame");
}

inline void Render::dispose()
{
	this->window.dispose();
}


RenderHandle create_render()
{
	return new Render();
};

void destroy_render(const RenderHandle& p_render)
{
	((Render*)p_render)->dispose();
	delete (Render*)p_render;
};

bool render_window_should_close(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	return rdwindow::window_should_close(l_render->window.Handle);
};

void render_window_pool_event(const RenderHandle& p_render)
{
	Render* l_render = (Render*)p_render;
	rdwindow::window_pool_event(l_render->window.Handle);
};