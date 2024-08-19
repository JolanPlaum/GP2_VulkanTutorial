#ifndef GP2VKT_MESH_H_
#define GP2VKT_MESH_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "DataTypes.h"
#include "Texture.h"
#include "RAII/GP2_VkBuffer.h"
#include "RAII/GP2_VkDeviceMemory.h"

// Class Forward Declarations
class GP2_SingleTimeCommand;


// Class Declaration
class Mesh final
{
public:
	// Constructors and Destructor
	explicit Mesh(VkDevice device, VkPhysicalDevice physicalDevice, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const char* filePath, std::vector<Texture>&& textures);
	~Mesh() = default;
	
	// Copy and Move semantics
	Mesh(const Mesh& other)					= delete;
	Mesh& operator=(const Mesh& other)		= delete;
	Mesh(Mesh&& other) noexcept				= delete;
	Mesh& operator=(Mesh&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	void Update(void* modelDst) const;
	void Render(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet descriptorSet, VkSampler sampler, void* modelDst) const;

	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float sx, float sy, float sz);

	glm::mat4 CalculateTransform() const;


private:
	// Member variables
	glm::vec3 m_Position{ 0.f, 0.f, 0.f };
	glm::vec3 m_Rotation{ 0.f, 0.f, 0.f };
	glm::vec3 m_Scale{ 1.f, 1.f, 1.f };

	VkDevice m_Device{ nullptr };

	std::vector<Texture> m_Textures{};

	uint32_t m_IndexCount{};
	VkDeviceSize m_IndexOffset{};
	GP2_VkBuffer m_VertexIndexBuffer{};
	GP2_VkDeviceMemory m_VertexIndexBufferMemory{};


	//---------------------------
	// Private Member Functions
	//---------------------------
	void UpdateModelUniformBuffer(void* modelDst) const;
	void UpdateDescriptorSets(VkDescriptorSet descriptorSet, VkSampler sampler) const;
	void CmdBindings(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet descriptorSet) const;

	template<typename VertexType, typename IndexType>
	void CreateVertexIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices);

	// Init static helper functions
	void LoadModel(const char* filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);
	void LoadModel(const char* filePath, std::vector<VertexPBR>& vertices, std::vector<uint32_t>& indices);

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	VkDeviceSize CreateStagingBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas);
	void CopyBuffer(std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

};

template<typename VertexType, typename IndexType>
inline void Mesh::CreateVertexIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
{
	// Calculate vertex + index buffer size
	VkDeviceSize verticesSize{ sizeof(vertices[0]) * vertices.size() };
	VkDeviceSize indicesSize{ sizeof(indices[0]) * indices.size() };
	VkDeviceSize bufferSize{};

	// Create staging buffer with assigned data
	GP2_VkBuffer stagingBuffer{};
	GP2_VkDeviceMemory stagingBufferMemory{};
	bufferSize = CreateStagingBuffer(
		physicalDevice, device,
		stagingBuffer, stagingBufferMemory,
		{ verticesSize,		indicesSize },
		{ vertices.data(),	indices.data() });

	// Create vertex index buffer
	CreateBuffer(
		physicalDevice, device,
		m_VertexIndexBuffer, m_VertexIndexBufferMemory,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Transfer staging buffer to vertex index buffer
	CopyBuffer(std::move(commandBuffer), stagingBuffer, m_VertexIndexBuffer, bufferSize);

	// Set index count & offset
	m_IndexCount = static_cast<uint32_t>(indices.size());
	m_IndexOffset = verticesSize;
}
#endif