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
#include <cstring>


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
	// Instance should be created first
	CreateInstance();
	SetupDebugMessenger();

	PickPhysicalDevice();
	CreateLogicalDevice();
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
	vkDestroyDevice(m_Device, nullptr);

	// Destroyed right before instance to allow for debug messages during cleanup
	if (config::EnableValidationLayers) DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

	// Instance should be cleaned up last
	vkDestroyInstance(m_Instance, nullptr);

	// GLFW window
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void HelloTriangleApplication::CreateInstance()
{
	// Checking for layer support
	if (config::EnableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Optional data about our application (could optimize our application)
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Not optional data about extensions & validation layers
	std::vector<const char*> extensions = GetRequiredExtensions();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo; // TODO: make a helper getter function
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (config::EnableValidationLayers) {
		// Layers info
		createInfo.enabledLayerCount = static_cast<uint32_t>(config::ValidationLayers.size());
		createInfo.ppEnabledLayerNames = config::ValidationLayers.data();

		// Debug info
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		// Layers info
		createInfo.enabledLayerCount = 0;

		// Debug info
		createInfo.pNext = nullptr;
	}
	
	// Checking for extension support
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		//std::cout << "available extensions:\n";
		//for (const auto& extensionProperties : availableExtensions)
		//{
		//	std::cout << '\t' << extensionProperties.extensionName << '\n';
		//}
		//std::cout << '\n';

		std::cout << "required extensions:\n";
		for (const auto& extensionName : extensions)
		{
			bool isSupported{ false };
			for (const auto& extensionProperties : availableExtensions)
			{
				if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
					isSupported = true;
					break;
				}
			}
			std::cout << '\t' << extensionName << (isSupported ? "\tSUPPORTED\n" : "\tNOT PRESENT\n");
		}
		std::cout << '\n';
	}

	// Create instance using specified data
	if (VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance); result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
bool HelloTriangleApplication::CheckValidationLayerSupport() const
{
	// Get the number of layers available
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Allocate enough space & get the available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if all the config::ValidationLayers are available
	for (const char* layerName : config::ValidationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions() const
{
	// Get the required GLFW extensions
	uint32_t glfwExtensionCount{ 0 };
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Store the required extensions in a vector
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Add conditional extensions to the vector
	if (config::EnableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void HelloTriangleApplication::SetupDebugMessenger()
{
	// Exit early if validation layers are disabled
	if (!config::EnableValidationLayers) return;

	// Info about messenger and its callback
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	// Create extension object
	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
void HelloTriangleApplication::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	// Empty any existing data
	createInfo = {};

	// Info about messenger and its callback
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = HelloTriangleApplication::DebugCallback;
	createInfo.pUserData = nullptr; // Optional // Example: pass a pointer to HelloTriangleApplication class
}
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	// Return boolean indicating if the Vulkan call that triggered the message should be aborted
	//  > Only used to test validation layers themselves
	//  > Should always return VK_FALSE
	return VK_FALSE;
}
VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	// Error if the function couldn't be loaded
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void HelloTriangleApplication::PickPhysicalDevice()
{
	// Get the number of graphics cards available
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	// Exit early if there are 0 devices with Vulkan support
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	// Allocate an array to hold all the device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	// TODO: PhysicalDevice go for the best option instead of the first one that works
	// Evaluate each device and check if they are suitable
	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device)) {
			m_PhysicalDevice = device;
			break;
		}
	}

	// Throw if none are suitable
	if (m_PhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}
bool HelloTriangleApplication::IsDeviceSuitable(VkPhysicalDevice device)
{
	/*
	// Basic device properties (e.g. name, type & supported Vulkan version)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Optional features (e.g. texture compression, 64 bit floats & multi viewport rendering)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	*/

	QueueFamilyIndices indices = FindQueueFamilies(device);

	return indices.IsComplete();
}
QueueFamilyIndices HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get the number of queue families
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// Allocate an array to hold all the available queue families
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// TODO: QueueFamily understand this code, IMPORTANT!!!
	// We need to find at least 1 queue family that supports 'VK_QUEUE_GRAPHICS_BIT'
	int i{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.GraphicsFamily = i;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

void HelloTriangleApplication::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

	// Priority between 0.0 and 1.0
	float queuePriority{ 1.0f };

	// Describes the number of queues for a single queue family
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.GraphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// TODO: DeviceFeatures make sure this is implemented for final version
	// Temporarily empty as we currently don't need anything special
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Main info structor for logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = 0;
	if (config::EnableValidationLayers) {
		// These are ignored by up-to-date implementations
		// Good idea to set them anyway for compatibility with older versions
		createInfo.enabledLayerCount = static_cast<uint32_t>(config::ValidationLayers.size());
		createInfo.ppEnabledLayerNames = config::ValidationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Create logical device using specified data
	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// Retrieve queue handle for queue family (index 0 as there's only one right now)
	vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
}
