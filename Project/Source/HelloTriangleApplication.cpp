//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "HelloTriangleApplication.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Utils.h"
#include "DataTypes.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>
#include <chrono>
#include <numeric>

#include "RAII/GP2_VkShaderModule.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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
	// Do not create OpenGL context 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Disable resizing
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create the window resource
	m_pWindow = std::make_unique<GP2_GLFWwindow>(config::WIDTH, config::HEIGHT, "Vulkan");
	glfwSetWindowUserPointer(static_cast<GLFWwindow*>(*m_pWindow), this);

	// Set up an explicit callback to detect resizes
	glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(*m_pWindow), FramebufferResizeCallback);
}
void HelloTriangleApplication::InitVulkan()
{
	// Instance should be created first
	CreateInstance();
	SetupDebugMessenger();
	m_pSurface = std::make_unique<GP2_VkSurfaceKHR>(*m_pInstance, static_cast<GLFWwindow*>(*m_pWindow)); // can affect physical device selection

	// Physical and logical device setup
	PickPhysicalDevice();
	CreateLogicalDevice();

	CreateRenderPass(ChooseSwapSurfaceFormat(QuerySwapChainSupport(m_PhysicalDevice, *m_pSurface).Formats).format);

	CreateSwapChain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();

	m_pDescriptorSetLayout = std::make_unique<GP2_VkDescriptorSetLayout>(*m_pDevice, std::vector{ GetLayoutBindingUBO(), GetLayoutBindingSampler() });
	m_pPipelineLayout = std::make_unique<GP2_VkPipelineLayout>(*m_pDevice, std::vector<VkDescriptorSetLayout>{ *m_pDescriptorSetLayout });
	CreateGraphicsPipeline();

	CreateVertexIndexBuffer(config::Vertices, config::Indices);
	CreateUniformBuffers();
	CreateTextureImage("Resources/Textures/texture.jpg", STBI_rgb_alpha);
	CreateTextureSampler();

	CreateDescriptorSets();
	UpdateDescriptorSets();

	CreateCommandPool();
	RecordCommandBuffers();

	CreateSyncObjects();
}
void HelloTriangleApplication::MainLoop()
{
	// While the window is still open
	while (!glfwWindowShouldClose(static_cast<GLFWwindow*>(*m_pWindow)))
	{
		glfwPollEvents();
		DrawFrame();
	}

	// Wait for operations to finish before exiting
	vkDeviceWaitIdle(*m_pDevice);
}
void HelloTriangleApplication::Cleanup()
{
	DestroySyncObjects();

	m_pCommandBuffers = nullptr;
	m_pDescriptorSets = nullptr;

	m_pTextureSampler = nullptr;
	m_pTextureImageView = nullptr;
	m_pTextureImageMemory = nullptr;
	m_pTextureImage = nullptr;

	m_pUniformBufferMemory = nullptr;
	m_pUniformBuffer = nullptr;
	m_pVertexIndexBufferMemory = nullptr;
	m_pVertexIndexBuffer = nullptr;

	m_pGraphicsPipeline = nullptr;
	m_pPipelineLayout = nullptr;
	m_pDescriptorSetLayout = nullptr;

	CleanupSwapChain();

	m_pRenderPass = nullptr;

	m_pDevice = nullptr;

	// Destroyed right before instance to allow for debug messages during cleanup
	if (config::EnableValidationLayers) m_pDebugMessenger = nullptr;

	// Instance should be cleaned up last
	m_pSurface = nullptr;
	m_pInstance = nullptr;

	// GLFW window
	m_pWindow = nullptr;
}

void HelloTriangleApplication::DrawFrame()
{
	// Wait for the previous frame to finish
	vkWaitForFences(*m_pDevice, 1, &static_cast<const VkFence&>(m_InFlightFences[m_CurrentFrame]), VK_TRUE, UINT64_MAX);

	// Acquire an image from the swap chain
	uint32_t imageIndex{};
	VkResult result = vkAcquireNextImageKHR(*m_pDevice, *m_pSwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
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
	vkResetFences(*m_pDevice, 1, &static_cast<const VkFence&>(m_InFlightFences[m_CurrentFrame]));

	// Update model-view-projection matrices
	UpdateUniformBuffer(m_CurrentFrame);

	// Record a command buffer which draws the scene onto the acquired image
	// Needed to update descriptor set data (model-view-projection UBO data)
	vkResetCommandBuffer(m_pCommandBuffers->Get()[imageIndex], 0);
	RecordCommandBuffer(m_pCommandBuffers->Get()[imageIndex], imageIndex);

	// Submit the recorded command buffer to the GPU
	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // corresponds to semaphore with same index
	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
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

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// Present the swap chain image to the screen
	VkPresentInfoKHR presentInfo{};
	{
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { *m_pSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex; // image for each swap chain in pSwapchains

		// Specify array of VkResults
		presentInfo.pResults = nullptr; // Optional
	}

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_IsFramebufferResized) {
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	// Advance to the next frame
	++m_CurrentFrame %= config::MAX_FRAMES_IN_FLIGHT;
}
void HelloTriangleApplication::UpdateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	// Calculate how much time has passed since start
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};

	// Model Matrix
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// View Matrix
	ubo.view = glm::lookAt(glm::vec3(0.0f, -3.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Projection Matrix
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChainExtent.width / (float)m_SwapChainExtent.height, 0.1f, 10.0f);

	// Compensate for inverted Y (GLM) by flipping the sign on Y axis in the projection matrix
	ubo.proj[1][1] *= -1;

	// Copy data to the mapped uniform buffer
	memcpy(m_MappedUniformBuffers[currentImage], &ubo, sizeof(ubo));
	/* TODO: MVP-MATRIX using a UBO this way is not the most efficient way to pass frequently changing values\
	to the shader. A more efficient way to pass a small buffer of data to shaders are push constants*/
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
	m_pDebugMessenger = std::make_unique<GP2_VkDebugUtilsMessengerEXT>(*m_pInstance, createInfo);
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

void HelloTriangleApplication::PickPhysicalDevice()
{
	// Get the number of graphics cards available
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(*m_pInstance, &deviceCount, nullptr);

	// Exit early if there are 0 devices with Vulkan support
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	// Allocate an array to hold all the device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(*m_pInstance, &deviceCount, devices.data());

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
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	bool isGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ||
		deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
		deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, *m_pSurface);
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	return isGPU && indices.IsComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *m_pSurface, &presentSupport);
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
	deviceFeatures.samplerAnisotropy = VK_TRUE; // TODO: match samplerAnisotropy with support for it by physical device through vkGetPhysicalDeviceFeatures


	// Create logical device using specified data
	m_pDevice = std::make_unique<GP2_VkDevice>(m_PhysicalDevice, queueCreateInfos, config::ValidationLayers, config::DeviceExtensions, deviceFeatures);

	// Retrieve queue handle for queue family (index 0 as there's only one right now)
	vkGetDeviceQueue(*m_pDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(*m_pDevice, indices.PresentFamily.value(), 0, &m_PresentQueue);
}

void HelloTriangleApplication::CreateSwapChain()
{
	// Get swap chain details for chosen physical device
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice, *m_pSurface);

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
		createInfo.surface = *m_pSurface;

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

	m_pSwapChain = std::make_unique<GP2_VkSwapchainKHR>(*m_pDevice, createInfo);

	// Get the final number of images
	vkGetSwapchainImagesKHR(*m_pDevice, *m_pSwapChain, &imageCount, nullptr);

	// Allocate an array to hold all the swap chain images
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(*m_pDevice, *m_pSwapChain, &imageCount, m_SwapChainImages.data());

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
		m_SwapChainImageViews.push_back({ *m_pDevice, m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT });
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
		std::vector<VkImageView> attachments = {
			m_SwapChainImageViews[i],
			*m_pDepthImageView, // Same depth image for all framebuffers because only 1 subpass running at the same time due to semaphores
		};

		// TODO: FrameBuffer get rid of magic numbers and pass values through function parameters
		//  layerCount refers to the number of layers inside of each swap chain image
		m_SwapChainFramebuffers.push_back({ *m_pDevice, *m_pRenderPass, attachments, m_SwapChainExtent, 1 });
	}
}
void HelloTriangleApplication::RecreateSwapChain()
{
	m_IsFramebufferResized = false;

	// Pause rendering while window is minimized
	int width = 0, height = 0;
	glfwGetFramebufferSize(static_cast<GLFWwindow*>(*m_pWindow), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(static_cast<GLFWwindow*>(*m_pWindow), &width, &height);
		glfwWaitEvents();
	}

	// Don't touch resources that are still in use
	// TODO: RecreateSwapChain use oldSwapChain to allow existing swap chain to operate
	//  while creating a new one (hint: https://vulkan-tutorial.com/en/Drawing_a_triangle/Swap_chain_recreation#page_Recreating-the-swap-chain )
	vkDeviceWaitIdle(*m_pDevice);

	// Ensure old versions are correctly cleaned up
	CleanupSwapChain();

	// Create swap chain and all of the objects that depend on it
	CreateSwapChain();
	CreateImageViews();
	CreateDepthResources();
	CreateFramebuffers();

	if (m_SwapChainImages.size() != m_pCommandBuffers->Get().size()) {
		CreateCommandPool();
	}
	RecordCommandBuffers();
}
void HelloTriangleApplication::CleanupSwapChain()
{
	m_pDepthImageView = nullptr;
	m_pDepthImageMemory = nullptr;
	m_pDepthImage = nullptr;

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
	glfwGetFramebufferSize(static_cast<GLFWwindow*>(*m_pWindow), &width, &height);

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
	VkAttachmentDescription colorAttachment{};
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
	VkAttachmentDescription depthAttachment{};
	{
		depthAttachment.format = FindDepthFormat(); // should match the format of the depth image itself
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no need to store depth data as it won't be used after drawing has finished

		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // we don't care about previous depth contents
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Attachment reference
	VkAttachmentReference colorAttachmentRef{};
	{
		colorAttachmentRef.attachment = 0; // specifies which attachment to reference (by index)

		// Vulkan will automatically transition the attachment to this layout when the subpass is started
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // dependant on the attachment it references
	}
	VkAttachmentReference depthAttachmentRef{};
	{
		depthAttachmentRef.attachment = 1;

		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		// Attachments that are not used by this subpass, but for which the data must be preserved
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;
	}

	// Subpass dependency
	VkSubpassDependency dependency{};
	{
		// Specify index of dependancy and dependant subpass
		// dstSubpass must always be higher than srcSubpass (exception: VK_SUBPASS_EXTERNAL)
		// VK_SUBPASS_EXTERNAL refers to implicit subpass before/after render pass depending on usage in [src|dst]SubPass
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // subpass this dependency depends on
		dependency.dstSubpass = 0; // subpass that is dependant on the dependancy

		// Specify stages to wait on (src) and which stages should wait on them (dst) 
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		// Specify operations to wait on (src) and which operations should wait on them (dst)
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		// The depth image is first accessed in _EARLY_FRAGMENT_TESTS_BIT and since depthAttachment.loadOp clears, specify _DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	}

	m_pRenderPass = std::make_unique<GP2_VkRenderPass>(*m_pDevice, std::vector{ colorAttachment, depthAttachment }, std::vector{ subpass }, std::vector{ dependency });
}

VkDescriptorSetLayoutBinding HelloTriangleApplication::GetLayoutBindingUBO()
{
	// Within 1 descriptor set layout binding has to be the same type of variable

	// Uniform Buffer Object layout
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	return uboLayoutBinding;
}
VkDescriptorSetLayoutBinding HelloTriangleApplication::GetLayoutBindingSampler()
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

	return samplerLayoutBinding;
}
void HelloTriangleApplication::CreateGraphicsPipeline()
{
	// Create shader modules locally (should be destroyed right after pipeline creation)
	GP2_VkShaderModule vertShaderModule{ *m_pDevice, config::VERTEX_SHADER_PATH };
	GP2_VkShaderModule fragShaderModule{ *m_pDevice, config::FRAGMENT_SHADER_PATH };

	// TODO: Shader read up on .pName and .pSpecializationInfo, interesting for custimization of single shader usage
	// Assign vertex & fragment shader to a specific pipeline stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // specify pipeline stage
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // function to invoke (entrypoint), allows for multiple shaders in 1 module
	vertShaderStageInfo.pSpecializationInfo = nullptr; // specify shader constants, allows for shader behavior configured at pipeline creation

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{
		Vertex3D::GetBindingDescription()
	};
	auto vertexAttributeDescriptionsArray = Vertex3D::GetAttributeDescriptions();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
		vertexAttributeDescriptionsArray.begin(),
		vertexAttributeDescriptionsArray.end()
	};

	// Describe the format of vertex data that will be passed to the vertex shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();



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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // specifies vertex order
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
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE; // specify if depth of new fragments should be compared to depth buffer to check if they should be discarded
	depthStencilState.depthWriteEnable = VK_TRUE; // specify if new depth of fragments that pass the test should be written to depth buffer
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS; // specify comparison operator that is performed during depth test
	depthStencilState.depthBoundsTestEnable = VK_FALSE; // if true fragments whose depth is outside [minDepthBounds,maxDepthBounds] will be discarded
	depthStencilState.minDepthBounds = 0.0f; // Optional
	depthStencilState.maxDepthBounds = 1.0f; // Optional
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {}; // Optional
	depthStencilState.back = {}; // Optional



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
	pipelineInfo.pDepthStencilState = &depthStencilState; // Optional (unless renderpass contains a depth stencil attachment)
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = *m_pPipelineLayout;
	pipelineInfo.renderPass = *m_pRenderPass;
	pipelineInfo.subpass = 0; // index of the subpass where this pipeline will be used
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional // needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag
	pipelineInfo.basePipelineIndex = -1; // Optional // needs VK_PIPELINE_CREATE_DERIVATIVE_BIT flag

	m_pGraphicsPipeline = std::make_unique<GP2_VkPipeline>(*m_pDevice, pipelineInfo);
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
	if (vkCreateShaderModule(*m_pDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void HelloTriangleApplication::CreateCommandPool()
{
	// TODO: QueueFamilies find out why we find new queue families indices every time instead of storing the indices
	m_pCommandBuffers = std::make_unique<PoolCommandBuffers>(
		*m_pDevice,
		FindQueueFamilies(m_PhysicalDevice).GraphicsFamily.value(),
		static_cast<uint32_t>(m_SwapChainImages.size()));
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

		
	// TODO: match clear values in RecordCommandBuffer to the attachments in render pass attachments
	std::vector<VkClearValue> clearValues{};
	clearValues.resize(2);
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	// Configure render pass before drawing
	VkRenderPassBeginInfo renderPassInfo{};
	{
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = *m_pRenderPass;
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

		// Define where shader loads/stores take place, should match size of attachments for best performance
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainExtent;

		// Define values to use for VK_ATTACHMENT_LOAD_OP_CLEAR (used in attachments)
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
	}

	// Render pass
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// Bind graphics pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pGraphicsPipeline);

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

		// Bind index buffer
		vkCmdBindIndexBuffer(commandBuffer, *m_pVertexIndexBuffer, sizeof(config::Vertices[0]) * config::Vertices.size(), VK_INDEX_TYPE_UINT16);

		// Bind the right descriptor set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipelineLayout, 0, 1, &m_pDescriptorSets->Get()[m_CurrentFrame], 0, nullptr);

		{
			// Bind vertex buffer
			std::vector<VkBuffer> vertexBuffers{ *m_pVertexIndexBuffer };
			std::vector<VkDeviceSize> offsets{ 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());

			// TODO: CmdDraw get rid of magic numbers
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(config::Indices.size()), 1, 0, 0, 0);
		}
	}
	vkCmdEndRenderPass(commandBuffer);

	// End recording commands
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
std::unique_ptr<PoolCommandBuffers> HelloTriangleApplication::BeginSingleTimeCommands()
{
	// Temporary command buffer
	auto pCommandBuffer = std::make_unique<PoolCommandBuffers>(
		*m_pDevice,
		FindQueueFamilies(m_PhysicalDevice).GraphicsFamily.value(), // TODO: use a transfer family queue
		1
	);

	// Specify command buffer usage
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Begin recording commands
	vkBeginCommandBuffer(pCommandBuffer->Get()[0], &beginInfo);

	return std::move(pCommandBuffer);
}
void HelloTriangleApplication::EndSingleTimeCommands(std::unique_ptr<PoolCommandBuffers> pCommandBuffer)
{
	// End recording commands
	vkEndCommandBuffer(pCommandBuffer->Get()[0]);

	// Execute the commands
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &pCommandBuffer->Get()[0];

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_GraphicsQueue);
}

void HelloTriangleApplication::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkImage& image, GP2_VkDeviceMemory& imageMemory)
{
	// Create image resource
	image = std::move(GP2_VkImage{ *m_pDevice, format, { width, height, 1 } , tiling, usage, false });

	// Create image memory requirements
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(*m_pDevice, image, &memRequirements);

	// Allocate image memory resource
	imageMemory = std::move(GP2_VkDeviceMemory{ *m_pDevice, memRequirements.size, FindMemoryType(memRequirements.memoryTypeBits, properties) });

	// Associate memory with image
	vkBindImageMemory(*m_pDevice, image, imageMemory, 0);
}
void HelloTriangleApplication::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory)
{
	// Create buffer resource
	buffer = std::move(GP2_VkBuffer{ *m_pDevice, size, usage, false });

	// Get buffer memory requirements
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(*m_pDevice, buffer, &memRequirements);

	// Allocate buffer memory resource
	bufferMemory = std::move(GP2_VkDeviceMemory{ *m_pDevice, memRequirements.size, FindMemoryType(memRequirements.memoryTypeBits, properties) });

	// Associate memory with buffer
	vkBindBufferMemory(*m_pDevice, buffer, bufferMemory, 0);

}
void HelloTriangleApplication::CreateUniformBuffers()
{
	VkDeviceSize bufferSize{ sizeof(UniformBufferObject) };
	size_t bufferCount{ config::MAX_FRAMES_IN_FLIGHT };

	m_pUniformBuffer = std::make_unique<GP2_VkBuffer>();
	m_pUniformBufferMemory = std::make_unique<GP2_VkDeviceMemory>();
	m_MappedUniformBuffers.resize(bufferCount);

	CreateBuffer(
		bufferSize * bufferCount,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		*m_pUniformBuffer,
		*m_pUniformBufferMemory);

	void* data{};
	vkMapMemory(*m_pDevice, *m_pUniformBufferMemory, 0, bufferSize * bufferCount, 0, &data);

	char* byteData = static_cast<char*>(data);
	for (size_t i{ 0 }; i < bufferCount; ++i)
	{
		m_MappedUniformBuffers[i] = (byteData + bufferSize * i);
	}
}
void HelloTriangleApplication::CreateDepthResources()
{
	// Find a depth format
	VkFormat depthFormat = FindDepthFormat();

	// Create image
	m_pDepthImage = std::make_unique<GP2_VkImage>();
	m_pDepthImageMemory = std::make_unique<GP2_VkDeviceMemory>();
	CreateImage(
		m_SwapChainExtent.width, // Should have the same resolution as the color attachment
		m_SwapChainExtent.height, // Should have the same resolution as the color attachment
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		*m_pDepthImage,
		*m_pDepthImageMemory);

	// Create image view
	m_pDepthImageView = std::make_unique<GP2_VkImageView>(*m_pDevice, *m_pDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	// Completely optional as the render pass will take care of this transition
	TransitionImageLayout(*m_pDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
void HelloTriangleApplication::CreateTextureImage(const char* filePath, int nrChannels)
{
	// Load image from the given file path
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, nrChannels);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	// Calculate amount of pixels (bytes)
	VkDeviceSize imageSize = texWidth * texHeight * nrChannels;

	// Create staging buffer with assigned data
	GP2_VkBuffer stagingBuffer{};
	GP2_VkDeviceMemory stagingBufferMemory{};
	CreateStagingBuffer(stagingBuffer, stagingBufferMemory,
		{ imageSize },
		{ pixels });

	// Create image
	m_pTextureImage = std::make_unique<GP2_VkImage>();
	m_pTextureImageMemory = std::make_unique<GP2_VkDeviceMemory>();
	CreateImage(
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		*m_pTextureImage,
		*m_pTextureImageMemory);

	// Transfer staging buffer to texture image
	// TODO: combine operations in a single command buffer instead of waiting for queue to become idle every time
	TransitionImageLayout(*m_pTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, *m_pTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	TransitionImageLayout(*m_pTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Create image view
	m_pTextureImageView = std::make_unique<GP2_VkImageView>(*m_pDevice, *m_pTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	// Release resources
	stbi_image_free(pixels);
}
void HelloTriangleApplication::CreateTextureSampler()
{
	// Physical device properties
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

	// Create sampler resource
	m_pTextureSampler = std::make_unique<GP2_VkSampler>(*m_pDevice, VK_SAMPLER_ADDRESS_MODE_REPEAT, properties.limits.maxSamplerAnisotropy);
}
void HelloTriangleApplication::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	// Temporary command buffer
	std::unique_ptr<PoolCommandBuffers> pCommandBuffer{ BeginSingleTimeCommands() };


	// Use image memory barrier to perform layout transition
	VkImageMemoryBarrier barrier{};
	{
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout; // Use UNDEFINED if you don't care about existing contents of the image
		barrier.newLayout = newLayout;

		// If the barrier is used to transfer queue family ownership
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Set to IGNORED if not used
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Set to IGNORED if not used

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// Specifies operations that must happen before barrier and operations that must wait on the barrier
		barrier.srcAccessMask = 0; // Set seperately below
		barrier.dstAccessMask = 0; // Set seperately below
	}

	// Set subresource aspect mask
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT * HasStencilComponent(format);
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Set stage & access masks
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		// DepthBuffer will be read from (depth tests) and written to (new fragment drawn)
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// Reading happens in _EARLY_FRAGMENT_TESTS_BIT and writing in _LATE_FRAGMENT_TESTS_BIT
		// The earliest should be specified so that the DepthBuffer is ready for usage when it needs to be
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	// Pipeline barrier command
	vkCmdPipelineBarrier(
		pCommandBuffer->Get()[0],
		sourceStage, destinationStage,
		0, // dependency flags, either 0 or VK_DEPENDENCY_BY_REGION_BIT
		0, nullptr, // memory barriers
		0, nullptr, // buffer memory barriers
		1, &barrier // image memory barriers
	);


	// End recording & execute commands
	EndSingleTimeCommands(std::move(pCommandBuffer));
}
VkDeviceSize HelloTriangleApplication::CreateStagingBuffer(GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas)
{
	// Exit early in case of wrong input values
	if (sizes.size() != datas.size() || sizes.empty()) return 0;

	// Calculate total buffer size
	VkDeviceSize bufferSize = std::accumulate(sizes.begin(), sizes.end(), static_cast<VkDeviceSize>(0));

	// Create staging buffer
	CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// Copy datas to staging buffer
	void* dstData{};
	VkDeviceSize offset{ 0 };
	for (size_t i{ 0 }; i < datas.size(); ++i)
	{
		vkMapMemory(*m_pDevice, stagingBufferMemory, offset, sizes[i], 0, &dstData);
		memcpy(dstData, datas[i], sizes[i]);
		vkUnmapMemory(*m_pDevice, stagingBufferMemory);

		offset += sizes[i];
	}

	// Return the size of the staging buffer
	return bufferSize;
}
void HelloTriangleApplication::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	// Temporary command buffer
	std::unique_ptr<PoolCommandBuffers> pCommandBuffer{ BeginSingleTimeCommands() };

	// Copy buffer command
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(pCommandBuffer->Get()[0], srcBuffer, dstBuffer, 1, &copyRegion);

	// End recording & execute commands
	EndSingleTimeCommands(std::move(pCommandBuffer));
}
void HelloTriangleApplication::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	// Temporary command buffer
	std::unique_ptr<PoolCommandBuffers> pCommandBuffer{ BeginSingleTimeCommands() };

	// Image copy info
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	// Indicate in which parts of the image we want to copy pixels
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	// Copy buffer image command
	vkCmdCopyBufferToImage(
		pCommandBuffer->Get()[0],
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// End recording & execute commands
	EndSingleTimeCommands(std::move(pCommandBuffer));
}
uint32_t HelloTriangleApplication::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	// Available types of memory
	VkPhysicalDeviceMemoryProperties memProperties{};
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

	// Find a memory type that is suitable for the buffer
	for (uint32_t i{ 0 }; i < memProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
VkFormat HelloTriangleApplication::FindDepthFormat()
{
	// Order of formats decides preference
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}
VkFormat HelloTriangleApplication::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		// Query format properties
		VkFormatProperties props{};
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

		// Check if the features are supported for the given tiling mode
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	// Also possible to return a special value instead of throwing
	throw std::runtime_error("failed to find supported format!");
}
bool HelloTriangleApplication::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D16_UNORM_S8_UINT
		|| format ==  VK_FORMAT_S8_UINT;
}

void HelloTriangleApplication::CreateDescriptorSets()
{
	uint32_t descriptorSetCount{ config::MAX_FRAMES_IN_FLIGHT };

	// Describes which descriptor type(s) are used and how many of each type
	std::vector<VkDescriptorPoolSize> poolSizes{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 * descriptorSetCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 * descriptorSetCount }
	};

	// Create descriptor pool resource
	m_pDescriptorSets = std::make_unique<PoolDescriptorSets>(
		*m_pDevice,
		poolSizes,
		*m_pDescriptorSetLayout,
		descriptorSetCount);
}
void HelloTriangleApplication::UpdateDescriptorSets()
{
	// Populate every descriptor set
	for (size_t i{}; i < m_pDescriptorSets->Get().size(); ++i)
	{
		// Descriptors that refer to buffers are configured with a VkDescriptorBufferInfo struct
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = *m_pUniformBuffer;
		bufferInfo.range = sizeof(UniformBufferObject);
		bufferInfo.offset = bufferInfo.range * i;

		// Image info
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = *m_pTextureImageView;
		imageInfo.sampler = *m_pTextureSampler;

		// The configuration of descriptors
		std::vector<VkWriteDescriptorSet> descriptorWrites{ 2 };
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_pDescriptorSets->Get()[i]; // Descriptor set to update
		descriptorWrites[0].dstBinding = 0; // Binding index
		descriptorWrites[0].dstArrayElement = 0; // Descriptors can be arrays, in which case this specifies start idx
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1; // Should match the number of elements in either pImageInfo, pBufferInfo or pTexelBufferView
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_pDescriptorSets->Get()[i]; // Descriptor set to update
		descriptorWrites[1].dstBinding = 1; // Binding index
		descriptorWrites[1].dstArrayElement = 0; // Descriptors can be arrays, in which case this specifies start idx
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1; // Should match the number of elements in either pImageInfo, pBufferInfo or pTexelBufferView
		descriptorWrites[1].pImageInfo = &imageInfo;

		// Update the configuration of the descriptor(s)
		vkUpdateDescriptorSets(*m_pDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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
			m_ImageAvailableSemaphores.push_back({ *m_pDevice });
			m_RenderFinishedSemaphores.push_back({ *m_pDevice });

			// Start fences signaled so the first draw call isn't blocked
			m_InFlightFences.push_back({ *m_pDevice, true});
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
