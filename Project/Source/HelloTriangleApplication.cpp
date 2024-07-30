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

#include "RAII/GP2_VkShaderModule.h"


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

	// Disable resizing
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create the actual window
	m_Window = glfwCreateWindow(config::WIDTH, config::HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);

	// Set up an explicit callback to detect resizes
	glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
}
void HelloTriangleApplication::InitVulkan()
{
	// Instance should be created first
	CreateInstance();
	SetupDebugMessenger();
	m_pSurface = std::make_unique<GP2_VkSurfaceKHR>(m_pInstance->Get(), m_Window); // can affect physical device selection

	// Physical and logical device setup
	PickPhysicalDevice();
	CreateLogicalDevice();

	CreateRenderPass(ChooseSwapSurfaceFormat(QuerySwapChainSupport(m_PhysicalDevice, m_pSurface->Get()).Formats).format);

	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();

	m_pPipelineLayout = std::make_unique<GP2_VkPipelineLayout>(m_pDevice->Get());
	CreateGraphicsPipeline();

	CreateCommandPool();
	RecordCommandBuffers();

	CreateSyncObjects();
}
void HelloTriangleApplication::MainLoop()
{
	// While the window is still open
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	// Wait for operations to finish before exiting
	vkDeviceWaitIdle(m_pDevice->Get());
}
void HelloTriangleApplication::Cleanup()
{
	DestroySyncObjects();

	m_pCommandBuffers = nullptr;

	vkDestroyPipeline(m_pDevice->Get(), m_GraphicsPipeline, nullptr);
	m_pPipelineLayout = nullptr;

	CleanupSwapChain();

	m_pRenderPass = nullptr;

	m_pDevice = nullptr;

	// Destroyed right before instance to allow for debug messages during cleanup
	if (config::EnableValidationLayers) DestroyDebugUtilsMessengerEXT(m_pInstance->Get(), m_DebugMessenger, nullptr);

	// Instance should be cleaned up last
	m_pSurface = nullptr;
	m_pInstance = nullptr;

	// GLFW window
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void HelloTriangleApplication::DrawFrame()
{
	// Wait for the previous frame to finish
	vkWaitForFences(m_pDevice->Get(), 1, &m_InFlightFences[m_CurrentFrame].Get(), VK_TRUE, UINT64_MAX);

	// Acquire an image from the swap chain
	uint32_t imageIndex{};
	VkResult result = vkAcquireNextImageKHR(m_pDevice->Get(), m_pSwapChain->Get(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame].Get(), VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_IsFramebufferResized = false;
		RecreateSwapChain();
		std::cout << "VK_ERROR_OUT_OF_DATE_KHR\n";
		return; // TODO: RecreateSwapChain don't quit drawing a frame (look inside function for more info)
	}
	else if (result == VK_SUBOPTIMAL_KHR) {
		std::cout << "VK_SUBOPTIMAL_KHR\n";
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Reset fence if an image was succesfully acquired
	vkResetFences(m_pDevice->Get(), 1, &m_InFlightFences[m_CurrentFrame].Get());

	// Submit the recorded command buffer to the GPU
	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame].Get() };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // corresponds to semaphore with same index
	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame].Get() };
	VkSubmitInfo submitInfo{};
	{
		// TODO: SubmitCommands look into linking waitStages to RenderPass stages automatically
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pCommandBuffers->Get()[imageIndex];

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
	}

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame].Get()) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// Present the swap chain image to the screen
	VkPresentInfoKHR presentInfo{};
	{
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_pSwapChain->Get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex; // image for each swap chain in pSwapchains

		// Specify array of VkResults
		presentInfo.pResults = nullptr; // Optional
	}

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_IsFramebufferResized) {
		m_IsFramebufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	// Advance to the next frame
	++m_CurrentFrame %= config::MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	// Only use reinterpret_cast 
	HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->m_IsFramebufferResized = true;
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

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	PopulateDebugMessengerCreateInfo(debugCreateInfo);
	
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

	// Create instance resource
	m_pInstance = std::make_unique<GP2_VkInstance>(appInfo, extensions, config::EnableValidationLayers, config::ValidationLayers, &debugCreateInfo);
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
	if (CreateDebugUtilsMessengerEXT(m_pInstance->Get(), &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
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
	createInfo.messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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
	vkEnumeratePhysicalDevices(m_pInstance->Get(), &deviceCount, nullptr);

	// Exit early if there are 0 devices with Vulkan support
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	// Allocate an array to hold all the device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_pInstance->Get(), &deviceCount, devices.data());

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
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, m_pSurface->Get());
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_pSurface->Get(), &presentSupport);
		if (presentSupport) {
			indices.PresentFamily = i;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;

	/* TODO: FindQueueFamilies add logic to explicitly prefer a physical device that \
	supports drawing and presentation in the same queue for improved performance */
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
	m_pDevice = std::make_unique<GP2_VkDevice>(m_PhysicalDevice, queueCreateInfos, config::ValidationLayers, config::DeviceExtensions, deviceFeatures);

	// Retrieve queue handle for queue family (index 0 as there's only one right now)
	vkGetDeviceQueue(m_pDevice->Get(), indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_pDevice->Get(), indices.PresentFamily.value(), 0, &m_PresentQueue);
}

void HelloTriangleApplication::CreateSwapChain()
{
	// Get swap chain details for chosen physical device
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, m_pSurface->Get());

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
		createInfo.surface = m_pSurface->Get();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // this is always 1 unless you are developing a stereoscopic 3D application
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // possible to render to seperate image first (e.g. post-processing)

		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
		std::vector<uint32_t> queueFamilyIndices = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		// Using concurrent mode if the queue families differ & exclusive if they are the same
		// This way there's no need for explicit ownership transfer
		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
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

	m_pSwapChain = std::make_unique<GP2_VkSwapchainKHR>(m_pDevice->Get(), createInfo);

	// Get the final number of images
	vkGetSwapchainImagesKHR(m_pDevice->Get(), m_pSwapChain->Get(), &imageCount, nullptr);

	// Allocate an array to hold all the swap chain images
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_pDevice->Get(), m_pSwapChain->Get(), &imageCount, m_SwapChainImages.data());

	// Store the format and extent that was chosen, these will be needed later on
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
}
void HelloTriangleApplication::CreateImageViews()
{
	// Allocate enough space for all the swap chain image views
	m_SwapChainImageViews.reserve(m_SwapChainImages.size());

	// Loop over all the swap chain images
	for (size_t i{ 0 }; i < m_SwapChainImages.size(); ++i)
	{
		m_SwapChainImageViews.push_back({ m_pDevice->Get(), m_SwapChainImages[i], m_SwapChainImageFormat });
	}
}
void HelloTriangleApplication::CreateFramebuffers()
{
	// Allocate enough space for all the swap chain frame buffers
	m_SwapChainFramebuffers.reserve(m_SwapChainImageViews.size());

	// Loop over all the swap chain image views
	for (size_t i{ 0 }; i < m_SwapChainImageViews.size(); ++i)
	{
		// This should be bound to the respective attachment descriptions in the render pass
		std::vector<VkImageView> attachments = { m_SwapChainImageViews[i].Get() };

		// TODO: FrameBuffer get rid of magic numbers and pass values through function parameters
		//  layerCount refers to the number of layers inside of each swap chain image
		m_SwapChainFramebuffers.push_back({ m_pDevice->Get(), m_pRenderPass->Get(), attachments, m_SwapChainExtent, 1 });
	}
}
void HelloTriangleApplication::RecreateSwapChain()
{
	// Pause rendering while window is minimized
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_Window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	// Don't touch resources that are still in use
	// TODO: RecreateSwapChain use oldSwapChain to allow existing swap chain to operate
	//  while creating a new one (hint: https://vulkan-tutorial.com/en/Drawing_a_triangle/Swap_chain_recreation#page_Recreating-the-swap-chain )
	vkDeviceWaitIdle(m_pDevice->Get());

	// Ensure old versions are correctly cleaned up
	CleanupSwapChain();

	// Create swap chain and all of the objects that depend on it
	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();

	if (m_SwapChainImages.size() != m_pCommandBuffers->Get().size()) {
		CreateCommandPool();
	}
	RecordCommandBuffers();
}
void HelloTriangleApplication::CleanupSwapChain()
{
	m_SwapChainFramebuffers.clear();

	m_SwapChainImageViews.clear();

	m_pSwapChain = nullptr;
}
SwapChainSupportDetails HelloTriangleApplication::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	// Basic surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

	// Surface formats
	{
		// Get the number of formats available
		uint32_t formatCount{ 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		// Allocate enough space & get available formats
		if (formatCount != 0) {
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
		}
	}

	// Presentation modes
	{
		// Get the number of present modes available
		uint32_t presentModeCount{ 0 };
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		// Allocate enough space & get available present modes
		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
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
	// Exit early if it has already been filled in (max is the default value)
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

void HelloTriangleApplication::CreateRenderPass(VkFormat format)
{
	// There can be multiple attachments descriptions per render pass.
	// For every attachment there's 1 attachment reference, however
	//  this can be more if the desired reference layout differs.
	// Every subpass references one or more attachments


	// Attachment description
	VkAttachmentDescription colorAttachment{}; // normally an array but we're currently using a single color buffer
	{
		colorAttachment.format = format; // should match the format of the swap chain images
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // should match multisampling settings from graphics pipeline

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // determines what to do with data before rendering
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // determines what to do with data after rendering

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // loadOp but for stencil data
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // storeOp but for stencil data

		// These 2 values will be revisited later
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // specifies which layout the image will have before render pass begins
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // specifies the layout to automatically transition to after render pass finishes
	}

	// Attachment reference
	VkAttachmentReference colorAttachmentRef{}; // normally an array but we're currently using a single color buffer
	{
		colorAttachmentRef.attachment = 0; // specifies which attachment to reference (by index)

		// Vulkan will automatically transition the attachment to this layout when the subpass is started
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // dependant on the attachment it references
	}


	// Subpass description
	VkSubpassDescription subpass{}; // normally an array but we're currently using a single color buffer
	{
		// Be explicit about this being a graphics subpass as Vulkan may support compute subpasses in the future
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		// Each element corresponds to an output location in the fragment shader
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pResolveAttachments = nullptr; // used for multisampling (pColorAttachments can not be null)

		// Each element corresponds to an input attachment index in a fragment shader
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;

		// Attachment for depth and stencil data (single element)
		subpass.pDepthStencilAttachment = nullptr;

		// Attachments that are not used by this subpass, but for which the data must be preserved
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;
	}

	// Subpass dependency
	VkSubpassDependency dependency{};
	{
		// Specify index of dependancy and dependant subpass
		// dstSubpass must always be higher than srcSubpass (exception: VK_SUBPASS_EXTERNAL)
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // subpass this dependency depends on
		dependency.dstSubpass = 0; // subpass that is dependant on the dependancy

		// ???
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// ???
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	m_pRenderPass = std::make_unique<GP2_VkRenderPass>(m_pDevice->Get(), std::vector{ colorAttachment }, std::vector{ subpass }, std::vector{ dependency });
}

void HelloTriangleApplication::CreateGraphicsPipeline()
{
	// Create shader modules locally (should be destroyed right after pipeline creation)
	GP2_VkShaderModule vertShaderModule{ m_pDevice->Get(), "Shaders/shader.vert.spv" };
	GP2_VkShaderModule fragShaderModule{ m_pDevice->Get(), "Shaders/shader.frag.spv" };

	// TODO: Shader read up on .pName and .pSpecializationInfo, interesting for custimization of single shader usage
	// Assign vertex & fragment shader to a specific pipeline stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // specify pipeline stage
	vertShaderStageInfo.module = vertShaderModule.Get();
	vertShaderStageInfo.pName = "main"; // function to invoke (entrypoint), allows for multiple shaders in 1 module
	vertShaderStageInfo.pSpecializationInfo = nullptr; // specify shader constants, allows for shader behavior configured at pipeline creation

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule.Get();
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// Describe the format of vertex data that will be passed to the vertex shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;



	// Describe the kind of geometry drawn from vertices (topology) & if primitive restart is enabled
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport & scissor data (in case not dynamically used)
	/*
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_SwapChainExtent.width;
	viewport.height = (float)m_SwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;
	*/
	// Specify viewport & scissor count at pipeline creation
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr; // &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr; // &scissor;

	// Rasterizer takes geometry from vertex shader and turns it into fragments
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // if true fragments beyond near/far planes are clamped instead of discarded (requires GPU feature)
	rasterizer.rasterizerDiscardEnable = VK_FALSE; // if true geometry never passes through rasterizer stage (basically disables output to framebuffer)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // determines how fragments are generated for geometry (requires GPU feature if not fill)
	rasterizer.lineWidth = 1.0f; // describes thickness of lines (requires GPU feature if thicker than 1.0f)
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // determines type of culling
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // specifies vertex order
	rasterizer.depthBiasEnable = VK_FALSE; // if true allows depth values to be altered
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// Will be revisited later (requires GPU feature)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Needed in case of depth and/or stencil buffer
	//VkPipelineDepthStencilStateCreateInfo depthStencilState{};



	// Color blending configuration per attached framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // normally an array but there's currently only 1 framebuffer
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // resulting blended color is AND'd with
	colorBlendAttachment.blendEnable = VK_FALSE; // if false AND if logicOpEnable false then the fragment shader result color is unmodified
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Color blending settings for all colors
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; // if true allows usage of logicOp field
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // if not COPY then blendEnable (prev) will act as if false & colorWriteMask will be used to determine affected framebuffer channels
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;



	// Values that should have configuration ignored (these values NEED specification at draw time)
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	// Dynamic state info, only in case dynamic state is needed
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();



	// Combine all of the previous data structures to define the graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = nullptr; // Optional
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pPipelineLayout->Get();
	pipelineInfo.renderPass = m_pRenderPass->Get();
	pipelineInfo.subpass = 0; // index of the subpass where this pipeline will be used
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional // needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag
	pipelineInfo.basePipelineIndex = -1; // Optional // needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag

	// Create the graphics pipeline (can create multiple pipelines in a single call)
	if (vkCreateGraphicsPipelines(m_pDevice->Get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
VkShaderModule HelloTriangleApplication::CreateShaderModule(const std::vector<char>& code)
{
	// Specify bytecode and size
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	// Create shader module using specified data
	VkShaderModule shaderModule{};
	if (vkCreateShaderModule(m_pDevice->Get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void HelloTriangleApplication::CreateCommandPool()
{
	// TODO: QueueFamilies find out why we find new queue families indices every time instead of storing the indices
	m_pCommandBuffers = std::make_unique<GP2_CommandBuffers>(
		m_pDevice->Get(),
		FindQueueFamilies(m_PhysicalDevice).GraphicsFamily.value(),
		m_SwapChainImages.size());
}
void HelloTriangleApplication::RecordCommandBuffers()
{
	// Check if there are a correct amount of command buffers
	if (m_SwapChainImages.size() != m_pCommandBuffers->Get().size()) {
		throw std::runtime_error("failed to record command buffers due to size difference!");
	}

	// Record command buffer for each possible image
	for (int32_t i{}; i < m_SwapChainImages.size(); ++i)
	{
		vkResetCommandBuffer(m_pCommandBuffers->Get()[i], 0);
		RecordCommandBuffer(m_pCommandBuffers->Get()[i], i);
	}
}
void HelloTriangleApplication::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	// Specifies usage of command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // specifies how we're going to use the command buffer
	beginInfo.pInheritanceInfo = nullptr; // only relevant for secondary command buffers

	// Begin recording commands, if it was already recorder once it will implicitly reset it
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// Configure render pass before drawing
	VkRenderPassBeginInfo renderPassInfo{};
	{
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_pRenderPass->Get();
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex].Get();

		// Define where shader loads/stores take place, should match size of attachments for best performance
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainExtent;

		// Define colors to use for VK_ATTACHMENT_LOAD_OP_CLEAR (used in color attachment)
		std::vector<VkClearValue> clearColors = { VkClearValue{0.0f, 0.0f, 0.0f, 1.0f} };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderPassInfo.pClearValues = clearColors.data();
	}

	// Render pass
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// Bind graphics pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		// TODO: DynamicState make a big automatic switch to check the dynamic states of the given pipeline and set those values
		// SET DYNAMIC STATES !!!!! this will depend on what was chosen as dynamic state
		{
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(m_SwapChainExtent.width);
			viewport.height = static_cast<float>(m_SwapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_SwapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}

		// TODO: CmdDraw get rid of magic numbers
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}
	vkCmdEndRenderPass(commandBuffer);

	// End recording commands
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void HelloTriangleApplication::CreateSyncObjects()
{
	// Allocate enough space
	m_ImageAvailableSemaphores.reserve(config::MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.reserve(config::MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.reserve(config::MAX_FRAMES_IN_FLIGHT);

	// Create semaphores and fences
	for (uint32_t i{}; i < config::MAX_FRAMES_IN_FLIGHT; ++i)
	{
		try {
			m_ImageAvailableSemaphores.push_back({ m_pDevice->Get() });
			m_RenderFinishedSemaphores.push_back({ m_pDevice->Get() });

			// Start fences signaled so the first draw call isn't blocked
			m_InFlightFences.push_back({ m_pDevice->Get(), true});
		}
		catch (const std::exception& e) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
void HelloTriangleApplication::DestroySyncObjects()
{
	m_ImageAvailableSemaphores.clear();
	m_RenderFinishedSemaphores.clear();
	m_InFlightFences.clear();
}
