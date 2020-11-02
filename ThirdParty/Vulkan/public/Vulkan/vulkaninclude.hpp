#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef linux
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.hpp>
