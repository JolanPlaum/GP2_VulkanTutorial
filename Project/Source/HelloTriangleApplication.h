#ifndef GP2VKT_HELLOTRIANGLEAPPLICATION_H_
#define GP2VKT_HELLOTRIANGLEAPPLICATION_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>
#include <memory>
#include "DataTypes.h"
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
	std::unique_ptr<GP2_VkImage> m_pDepthImage;
	std::unique_ptr<GP2_VkDeviceMemory> m_pDepthImageMemory;
	std::unique_ptr<GP2_VkImageView> m_pDepthImageView;

	std::unique_ptr<GP2_VkDescriptorSetLayout> m_pDescriptorSetLayout;
	std::unique_ptr<GP2_VkPipelineLayout> m_pPipelineLayout;
	std::unique_ptr<GP2_VkPipeline> m_pGraphicsPipeline;

	// TODO: create a mesh object that holds Vertex/Index data
	// TODO: create a vertexindex buffer per mesh object
	// TODO: store offset for vertex and index in the buffer
	// TODO: make a single DeviceMemory for all the meshes
	std::vector<Vertex3D> m_ModelVertices;
	std::vector<uint32_t> m_ModelIndices;

	std::unique_ptr<GP2_VkBuffer> m_pVertexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pVertexBufferMemory;
	std::unique_ptr<GP2_VkBuffer> m_pIndexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pIndexBufferMemory;
	std::unique_ptr<GP2_VkBuffer> m_pVertexIndexBuffer;
	std::unique_ptr<GP2_VkDeviceMemory> m_pVertexIndexBufferMemory;

	std::unique_ptr<GP2_VkImage> m_pTextureImage; // Only used in CreateTextureImage
	std::unique_ptr<GP2_VkDeviceMemory> m_pTextureImageMemory; // Only used in CreateTextureImage
	std::unique_ptr<GP2_VkImageView> m_pTextureImageView; // Created in CreateTextureImage & referenced in UpdateDescriptorSets
	std::unique_ptr<GP2_VkSampler> m_pTextureSampler; // Created in CreateTextureSampler & referenced in UpdateDescriptorSets

	std::unique_ptr<GP2_VkBuffer> m_pUniformBuffer; // Created in CreateUniformBuffers & referenced in UpdateDescriptorSets
	std::unique_ptr<GP2_VkDeviceMemory> m_pUniformBufferMemory; // Only used in CreateUniformBuffers
	std::vector<void*> m_MappedUniformBuffers; // Created in CreateUniformBuffers & assigned in UpdateUniformBuffer
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

	void LoadModel(const char* filePath);

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkImage& image, GP2_VkDeviceMemory& imageMemory);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory);
	template <typename VertexType> void CreateVertexBuffer(const std::vector<VertexType>& vertices);
	template <typename IndexType> void CreateIndexBuffer(const std::vector<IndexType>& indices);
	template <typename VertexType, typename IndexType> void CreateVertexIndexBuffer(const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices);
	void CreateUniformBuffers();
	void CreateDepthResources();
	void CreateTextureImage(const char* filePath, int nrChannels);
	void CreateTextureSampler();
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkDeviceSize CreateStagingBuffer(GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool HasStencilComponent(VkFormat format);

	void CreateDescriptorSets();
	void UpdateDescriptorSets();

	void CreateSyncObjects();
	void DestroySyncObjects();
};

//---------------------------
// Template Member Functions
//---------------------------
template<typename VertexType>
inline void HelloTriangleApplication::CreateVertexBuffer(const std::vector<VertexType>& vertices)
{
	// Calculate vertex buffer size
	VkDeviceSize bufferByteSize{ sizeof(vertices[0]) * vertices.size() };

	// Create staging buffer with assigned data
	GP2_VkBuffer stagingBuffer{};
	GP2_VkDeviceMemory stagingBufferMemory{};
	CreateStagingBuffer(stagingBuffer, stagingBufferMemory,
		{ bufferByteSize },
		{ vertices.data() });

	// Create vertex buffer
	m_pVertexBuffer = std::make_unique<GP2_VkBuffer>();
	m_pVertexBufferMemory = std::make_unique<GP2_VkDeviceMemory>();
	CreateBuffer(
		bufferByteSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		*m_pVertexBuffer,
		*m_pVertexBufferMemory);

	// Transfer staging buffer to vertex buffer
	CopyBuffer(stagingBuffer, *m_pVertexBuffer, bufferByteSize);
}
template<typename IndexType>
inline void HelloTriangleApplication::CreateIndexBuffer(const std::vector<IndexType>& indices)
{
	// Calculate index buffer size
	VkDeviceSize bufferByteSize{ sizeof(indices[0]) * indices.size() };

	// Create staging buffer with assigned data
	GP2_VkBuffer stagingBuffer{};
	GP2_VkDeviceMemory stagingBufferMemory{};
	CreateStagingBuffer(stagingBuffer, stagingBufferMemory,
		{ bufferByteSize },
		{ indices.data() });

	// Create index buffer
	m_pIndexBuffer = std::make_unique<GP2_VkBuffer>();
	m_pIndexBufferMemory = std::make_unique<GP2_VkDeviceMemory>();
	CreateBuffer(
		bufferByteSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		*m_pIndexBuffer,
		*m_pIndexBufferMemory);

	// Transfer staging buffer to index buffer
	CopyBuffer(stagingBuffer, *m_pIndexBuffer, bufferByteSize);
}
template<typename VertexType, typename IndexType>
inline void HelloTriangleApplication::CreateVertexIndexBuffer(const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
{
	// Calculate vertex + index buffer size
	VkDeviceSize verticesSize{ sizeof(vertices[0]) * vertices.size() };
	VkDeviceSize indicesSize{ sizeof(indices[0]) * indices.size() };
	VkDeviceSize bufferSize{};

	// Create staging buffer with assigned data
	GP2_VkBuffer stagingBuffer{};
	GP2_VkDeviceMemory stagingBufferMemory{};
	bufferSize = CreateStagingBuffer(stagingBuffer, stagingBufferMemory,
		{ verticesSize,		indicesSize },
		{ vertices.data(),	indices.data() });

	// Create vertex index buffer
	m_pVertexIndexBuffer = std::make_unique<GP2_VkBuffer>();
	m_pVertexIndexBufferMemory = std::make_unique<GP2_VkDeviceMemory>();
	CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		*m_pVertexIndexBuffer,
		*m_pVertexIndexBufferMemory);

	// Transfer staging buffer to vertex index buffer
	CopyBuffer(stagingBuffer, *m_pVertexIndexBuffer, bufferSize);
}
#endif