#ifndef GP2VKT_HELLOTRIANGLEAPPLICATION_H_
#define GP2VKT_HELLOTRIANGLEAPPLICATION_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>
#include <memory>
#include "RAII/GP2_GLFWwindow.h"
#include "RAII/GP2_VkFence.h"
#include "RAII/GP2_VkSemaphore.h"
#include "RAII/GP2_VkCommandPool.h"
#include "RAII/GP2_VkPipelineLayout.h"
#include "RAII/GP2_VkPipeline.h"
#include "RAII/GP2_VkRenderPass.h"
#include "RAII/GP2_VkSwapchainKHR.h"
#include "RAII/GP2_VkImage.h"
#include "RAII/GP2_VkImageView.h"
#include "RAII/GP2_VkFramebuffer.h"
#include "RAII/GP2_VkSurfaceKHR.h"
#include "RAII/GP2_VkInstance.h"
#include "RAII/GP2_VkDevice.h"
#include "RAII/GP2_VkBuffer.h"
#include "RAII/GP2_VkDeviceMemory.h"
#include "RAII/GP2_VkDescriptorSetLayout.h"
#include "RAII/GP2_VkDescriptorPool.h"
#include "RAII/GP2_VkSampler.h"
#include "RAII/GP2_VkDebugUtilsMessengerEXT.h"
#include "PoolCommandBuffers.h"
#include "PoolDescriptorSets.h"

// Class Forward Declarations
struct GLFWwindow;
struct Vertex;
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
	std::unique_ptr<GP2_GLFWwindow> m_pWindow;

	std::unique_ptr<GP2_VkInstance> m_pInstance;
	std::unique_ptr<GP2_VkDebugUtilsMessengerEXT> m_pDebugMessenger;
	std::unique_ptr<GP2_VkSurfaceKHR> m_pSurface;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	std::unique_ptr<GP2_VkDevice> m_pDevice;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	std::unique_ptr<GP2_VkRenderPass> m_pRenderPass;

	std::unique_ptr<GP2_VkSwapchainKHR> m_pSwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<GP2_VkImageView> m_SwapChainImageViews;
	std::vector<GP2_VkFramebuffer> m_SwapChainFramebuffers;

	std::unique_ptr<GP2_VkDescriptorSetLayout> m_pDescriptorSetLayout;
	std::unique_ptr<GP2_VkPipelineLayout> m_pPipelineLayout;
	std::unique_ptr<GP2_VkPipeline> m_pGraphicsPipeline;

	// TODO: create a mesh object that holds Vertex/Index data
	// TODO: create a vertexindex buffer per mesh object
	// TODO: store offset for vertex and index in the buffer
	// TODO: make a single DeviceMemory for all the meshes
	std::unique_ptr<GP2_VkBuffer> m_pVertexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pVertexBufferMemory;
	std::unique_ptr<GP2_VkBuffer> m_pIndexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pIndexBufferMemory;
	std::unique_ptr<GP2_VkBuffer> m_pVertexIndexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pVertexIndexBufferMemory;
	std::unique_ptr<GP2_VkBuffer> m_pUniformBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pUniformBufferMemory;
	std::vector<void*> m_MappedUniformBuffers;

	std::unique_ptr<GP2_VkImage> m_pTextureImage;
	std::unique_ptr<GP2_VkDeviceMemory> m_pTextureImageMemory;
	std::unique_ptr<GP2_VkImageView> m_pTextureImageView;
	std::unique_ptr<GP2_VkSampler> m_pTextureSampler;

	std::unique_ptr<PoolDescriptorSets> m_pDescriptorSets;
	std::unique_ptr<PoolCommandBuffers> m_pCommandBuffers;

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
	void UpdateUniformBuffer(uint32_t currentImage);

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
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void CreateRenderPass(VkFormat format);

	static VkDescriptorSetLayoutBinding GetLayoutBindingUBO();
	static VkDescriptorSetLayoutBinding GetLayoutBindingSampler();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void CreateCommandPool();
	void RecordCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	std::unique_ptr<PoolCommandBuffers> BeginSingleTimeCommands();
	void EndSingleTimeCommands(std::unique_ptr<PoolCommandBuffers> pCommandBuffer);

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkImage& image, GP2_VkDeviceMemory& imageMemory);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory);
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<uint16_t>& indices);
	void CreateVertexIndexBuffer(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
	void CreateUniformBuffers();
	void CreateTextureImage(const char* filePath, int nrChannels);
	void CreateTextureSampler();
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkDeviceSize CreateStagingBuffer(GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateDescriptorSets();
	void UpdateDescriptorSets();

	void CreateSyncObjects();
	void DestroySyncObjects();
};
#endif