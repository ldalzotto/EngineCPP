#pragma once

#define USING_GLFW 1
#define NOMINMAX

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef linux
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <Vulkan/vulkaninclude.hpp>
#include "glfw3.h"
#include "glfw3native.h"
