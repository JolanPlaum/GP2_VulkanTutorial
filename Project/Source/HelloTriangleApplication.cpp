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
#include <set>
#include <limits>
#include <algorithm>


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
	CreateSurface(); // can affect physical device selection

	// Physical and logical device setup
	PickPhysicalDevice();
	CreateLogicalDevice();

	CreateSwapChain();
	CreateImageViews();

	CreateGraphicsPipeline();
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
	for (auto imageView : m_SwapChainImageViews) { vkDestroyImageView(m_Device, imageView, nullptr); }
	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
	vkDestroyDevice(m_Device, nullptr);

	// Destroyed right before instance to allow for debug messages during cleanup
	if (config::EnableValidationLayers) DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

	// Instance should be cleaned up last
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
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

void HelloTriangleApplication::CreateSurface()
{
	// Create surface
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
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
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
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

		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
		if (presentSupport) {
			indices.PresentFamily = i;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}
bool HelloTriangleApplication::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Get the number of extensions available
	uint32_t extensionCount{ 0 };
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// Allocate an array to hold all the available extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// Check if all the config::DeviceExtensions are available
	std::set<std::string> requiredExtensions(config::DeviceExtensions.begin(), config::DeviceExtensions.end());
	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void HelloTriangleApplication::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

	// Store queue families in a set to ensure unique queues
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	// Loop over all unique queue families and create data for each one
	float queuePriority{ 1.0f };
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		// Describes the number of queues for a single queue family
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// Add to the list
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// TODO: DeviceFeatures make sure this is implemented for final version
	// Temporarily empty as we currently don't need anything special
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Main info structor for logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(config::DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = config::DeviceExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

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
	vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);
}

void HelloTriangleApplication::CreateSwapChain()
{
	// Get swap chain details for chosen physical device
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

	// Choose the right settings for the swap chain
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

	// Request one more image than the minimum requirements, while not exceeding the maximum
	// This prevents waiting on the driver to complete internal operations
	uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
	if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
		imageCount = swapChainSupport.Capabilities.maxImageCount;
	}

	// Create swap chain data
	VkSwapchainCreateInfoKHR createInfo{};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // this is always 1 unless you are developing a stereoscopic 3D application
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // possible to render to seperate image first (e.g. post-processing)

		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		// Using concurrent mode if the queue families differ & exclusive if they are the same
		// This way there's no need for explicit ownership transfer
		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.Capabilities.currentTransform; // can be used to specify a certain transform (e.g. 90deg angle)
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // specifies if alpha channel is used for blending w/ other windows
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE; // it's possible the swap chain becomes invalid/unoptimized (e.g. window was resized)
	}

	// Create swap chain using specified data
	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// Get the final number of images
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);

	// Allocate an array to hold all the swap chain images
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

	// Store the format and extent that was chosen, these will be needed later on
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
}
void HelloTriangleApplication::CreateImageViews()
{
	// Allocate enough space for all the swap chain images
	m_SwapChainImageViews.resize(m_SwapChainImages.size());

	// Loop over all the swap chain images
	for (size_t i{ 0 }; i < m_SwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChainImages[i];

		// Specify how the image data should be interpreted
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SwapChainImageFormat;

		// Specify color channel swizzling
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Describe the image purpose & which part to access
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Create image view using specified data
		if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}
SwapChainSupportDetails HelloTriangleApplication::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Basic surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

	// Surface formats
	{
		// Get the number of formats available
		uint32_t formatCount{ 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		// Allocate enough space & get available formats
		if (formatCount != 0) {
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
		}
	}

	// Presentation modes
	{
		// Get the number of present modes available
		uint32_t presentModeCount{ 0 };
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		// Allocate enough space & get available present modes
		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.PresentModes.data());
		}
	}

	return details;
}
VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Go through available formats and look for preferred combination
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// Return first format if preferred combination wasn't found
	// This could be done better, but ok for now
	return availableFormats[0];
}
VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// Return preferred present mode if it's available
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	// FIFO is guaranteed to always be available
	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D HelloTriangleApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// Exit early if ???
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	VkExtent2D actualExtent{};

	// Query window resolution in pixels
	int width{}, height{};
	glfwGetFramebufferSize(m_Window, &width, &height);

	// Match size against minimum/maximum image extent
	actualExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void HelloTriangleApplication::CreateGraphicsPipeline() {

}
