#ifndef GP2VKT_UTILS_H_
#define GP2VKT_UTILS_H_
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <fstream>
#include "DataTypes.h"

namespace config
{
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	const std::string VERTEX_SHADER_PATH = "Resources/Shaders/shader.vert.spv";
	const std::string FRAGMENT_SHADER_PATH = "Resources/Shaders/shader.frag.spv";

	const std::vector<const char*> DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<const char*> ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif

	const float MODEL_OFFSET_X = 0.0f;
	const std::vector<Vertex3D> Vertices{
		// Standard square with an offset on the X axis
		{ {MODEL_OFFSET_X+ -0.5f,-0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },
		{ {MODEL_OFFSET_X+  0.5f,-0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f } },
		{ {MODEL_OFFSET_X+  0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f } },
		{ {MODEL_OFFSET_X+ -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f } },

		// Duplicate just below the standard square
		{ {MODEL_OFFSET_X+ -0.5f,-0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },
		{ {MODEL_OFFSET_X+  0.5f,-0.5f,-0.5f }, { 0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f } },
		{ {MODEL_OFFSET_X+  0.5f, 0.5f,-0.5f }, { 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f } },
		{ {MODEL_OFFSET_X+ -0.5f, 0.5f,-0.5f }, { 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f } }
	};

	const std::vector<Vertex3D> Vertices2{
		{ { 0.5f,-0.5f, 0.0f }, { 1.0f, 0.0f, 1.0f}, {-1.0f, 2.0f } },
		{ { 1.5f,-0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f}, { 2.0f, 2.0f } },
		{ { 1.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f}, { 2.0f,-1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f}, {-1.0f,-1.0f } }
	};

	const std::vector<uint16_t> Indices{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
	};
}

namespace util
{
	static std::vector<char> ReadFile(const std::string& filename)
	{
		// Create file stream
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		// Check if the file exists
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		// Determine file size and allocate enough space
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// Go to beginning and read all the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		// Close file before exiting the function
		file.close();
		return buffer;
	}
}

#endif
