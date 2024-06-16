#ifndef GP2VKT_UTILS_H_
#define GP2VKT_UTILS_H_
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace config
{
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	const std::vector<const char*> DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif
}

#endif
