#ifndef GP2VKT_HELLOTRIANGLEAPPLICATION_H_
#define GP2VKT_HELLOTRIANGLEAPPLICATION_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>

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

	VkSwapchainKHR m_SwapChain; // TODO: RAII
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	VkRenderPass m_RenderPass; // TODO: RAII
	VkPipelineLayout m_PipelineLayout; // TODO: RAII
	VkPipeline m_GraphicsPipeline; // TODO: RAII

	VkCommandPool m_CommandPool; // TODO: RAII
	std::vector<VkCommandBuffer> m_CommandBuffers;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores; // TODO: RAII
	std::vector<VkSemaphore> m_RenderFinishedSemaphores; // TODO: RAII
	std::vector<VkFence> m_InFlightFences; // TODO: RAII

	uint32_t m_CurrentFrame = 0;


	//---------------------------
	// Private Member Functions
	//---------------------------
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

	void DrawFrame();

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
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void CreateRenderPass();

	void CreatePipelineLayout();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void CreateCommandPool();
	void AllocateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void CreateSyncObjects();
	void DestroySyncObjects();
};
#endif