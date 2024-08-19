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

	void CreateVertexIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

	// Init static helper functions
	void LoadModel(const char* filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	VkDeviceSize CreateStagingBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas);
	void CopyBuffer(std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

};
#endif