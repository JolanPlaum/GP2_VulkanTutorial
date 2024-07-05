#ifndef GP2VKT_HELLOTRIANGLEAPPLICATION_H_
#define GP2VKT_HELLOTRIANGLEAPPLICATION_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>
#include <memory>
#include "Source/GP2_VkFence.h"
#include "Source/GP2_VkSemaphore.h"
#include "Source/GP2_VkCommandPool.h"
#include "Source/GP2_VkPipelineLayout.h"
#include "Source/GP2_VkRenderPass.h"
#include "Source/GP2_VkSwapchainKHR.h"

// Class Forward Declarations
class GLFWwindow;
struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsFamily;
	std::optional<uint32_t> PresentFamily;

	bool IsComplete() const {
		return GraphicsFamily.has_value()
			&& PresentFamily.has_value();
	}
};
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};


// Class Declaration
class HelloTriangleApplication final
{
public:
	// Constructors and Destructor
	explicit HelloTriangleApplication() = default;
	~HelloTriangleApplication() = default;
	
	// Copy and Move semantics
	HelloTriangleApplication(const HelloTriangleApplication& other)					= delete;
	HelloTriangleApplication& operator=(const HelloTriangleApplication& other)		= delete;
	HelloTriangleApplication(HelloTriangleApplication&& other) noexcept				= delete;
	HelloTriangleApplication& operator=(HelloTriangleApplication&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	void Run();


private:
	// Member variables
	GLFWwindow* m_Window; // TODO: RAII

	VkInstance m_Instance; // TODO: RAII
	VkDebugUtilsMessengerEXT m_DebugMessenger; // TODO: RAII
	VkSurfaceKHR m_Surface; // TODO: RAII

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE; // TODO: RAII
	VkDevice m_Device; // TODO: RAII

	VkQueue m_GraphicsQueue; // TODO: RAII
	VkQueue m_PresentQueue; // TODO: RAII

	std::unique_ptr<GP2_VkSwapchainKHR> m_pSwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews; // TODO: RAII
	std::vector<VkFramebuffer> m_SwapChainFramebuffers; // TODO: RAII

	std::unique_ptr<GP2_VkRenderPass> m_pRenderPass;
	std::unique_ptr<GP2_VkPipelineLayout> m_pPipelineLayout;
	VkPipeline m_GraphicsPipeline; // TODO: RAII

	std::unique_ptr<GP2_VkCommandPool> m_pCommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	std::vector<GP2_VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<GP2_VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<GP2_VkFence> m_InFlightFences;

	uint32_t m_CurrentFrame = 0;
	bool m_IsFramebufferResized = false;


	//---------------------------
	// Private Member Functions
	//---------------------------
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

	void DrawFrame();

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	void CreateInstance();
	bool CheckValidationLayerSupport() const;
	std::vector<const char*> GetRequiredExtensions() const;

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

	void CreateSurface();

	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	void CreateLogicalDevice();

	void CreateSwapChain();
	void CreateImageViews();
	void CreateFramebuffers();
	void RecreateSwapChain();
	void CleanupSwapChain();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void CreateRenderPass();

	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void CreateCommandPool();
	void AllocateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void CreateSyncObjects();
	void DestroySyncObjects();
};
#endif