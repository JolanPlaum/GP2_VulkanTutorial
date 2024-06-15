//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "HelloTriangleApplication.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Utils.h"
#include <stdexcept>
#include <vector>
#include <iostream>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void HelloTriangleApplication::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Cleanup();
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void HelloTriangleApplication::InitWindow()
{
	// Should be the very first call
	glfwInit();

	// Do not create OpenGL context 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// TODO: enable resizing in final version
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create the actual window
	m_Window = glfwCreateWindow(config::WIDTH, config::HEIGHT, "Vulkan", nullptr, nullptr);
}
void HelloTriangleApplication::InitVulkan()
{
	CreateInstance();
}
void HelloTriangleApplication::MainLoop()
{
	// While the window is still open
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
	}
}
void HelloTriangleApplication::Cleanup()
{
	// Instance should be cleaned up last
	vkDestroyInstance(m_Instance, nullptr);

	// GLFW window
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void HelloTriangleApplication::CreateInstance()
{
	// Optional data about our application (could optimize our application)
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Not optional
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0; // temporary, will be done later
	
	// Checking for extension support
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		//std::cout << "available extensions:\n";
		//for (const auto& extension : extensions)
		//{
		//	std::cout << '\t' << extension.extensionName << '\n';
		//}
		//std::cout << '\n';

		std::cout << "required extensions:\n";
		for (uint32_t i{}; i < glfwExtensionCount; ++i)
		{
			bool isSupported{ false };
			for (const auto& extension : extensions)
			{
				if (strcmp(glfwExtensions[i], extension.extensionName) == 0) {
					isSupported = true;
					break;
				}
			}
			std::cout << '\t' << glfwExtensions[i] << (isSupported ? "\tSUPPORTED\n" : "\tNOT PRESENT\n");
		}
		std::cout << '\n';
	}

	// Create instance using specified data
	if (VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}