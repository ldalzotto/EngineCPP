#pragma once

#include <GLFW/glfwinclude.h>
#include <string>

typedef GLFWwindow* WindowHandle;

struct rdwindow
{
	inline static WindowHandle create_window(const short int p_width, const short int p_height, const std::string& p_title)
	{
		return glfwCreateWindow(p_width, p_height, p_title.c_str(), NULL, NULL);
	};

	inline static void free_window(const WindowHandle& p_windowHandle)
	{
		glfwDestroyWindow(p_windowHandle);
	}

	inline static void window_register_resize_callback(const WindowHandle& p_windowHandle, void(*p_callback)(WindowHandle, int, int))
	{
		glfwSetWindowSizeCallback((GLFWwindow*)p_windowHandle, p_callback);
	}

	inline static bool window_should_close(const WindowHandle& p_windowHandle)
	{
		return glfwWindowShouldClose(p_windowHandle);
	}

	inline static void window_pool_event(const WindowHandle& p_windowhandle)
	{
		return glfwPollEvents();
	}

#ifdef _WIN32
	inline static HWND get_window_native(const WindowHandle& p_windowHandle)
	{
		return glfwGetWin32Window(p_windowHandle);
	}
#endif

#ifdef linux
    inline static Window get_window_native(const WindowHandle& p_windowHandle)
    {
        return glfwGetX11Window(p_windowHandle);
    };
#endif

};
