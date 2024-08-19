//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "Mesh.h"
#include <stdexcept>
#include <unordered_map>
#include <numeric>
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/glm.hpp>
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif
#include <tiny_obj_loader.h>
#include "RAII/GP2_SingleTimeCommand.h"


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Mesh::Mesh(VkDevice device, VkPhysicalDevice physicalDevice, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const char* filePath, std::vector<Texture>&& textures)
    : m_Device{ device }
    , m_Textures{ std::move(textures) }
{
    // Get vertices & indices data
    std::vector<Vertex3D> vertices{};
    std::vector<uint32_t> indices{};
    LoadModel(filePath, vertices, indices);

    // Store the data inside of a buffer
    CreateVertexIndexBuffer(physicalDevice, device, std::move(commandBuffer), vertices, indices);
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void Mesh::Update(void* modelDst) const
{
    UpdateModelUniformBuffer(modelDst);
}

void Mesh::Render(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet descriptorSet, VkSampler sampler, void* modelDst) const
{
    UpdateModelUniformBuffer(modelDst);
    UpdateDescriptorSets(descriptorSet, sampler);
    CmdBindings(commandBuffer, layout, descriptorSet);
}

glm::mat4 Mesh::CalculateTransform() const
{
    // Create translation matrix
    glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), m_Position);

    // Create rotation matrix
    glm::mat4 matRotX = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
    glm::mat4 matRotY = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
    glm::mat4 matRotZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
    glm::mat4 matRot = matRotZ * matRotY * matRotX;

    // Ceate scaling matrix
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f), m_Scale);

    // Combine into single transformation matrix and return that value
    return matTrans * matRot * matScale;
}

void Mesh::SetPosition(float x, float y, float z)
{
    m_Position = { x, y, z };
}
void Mesh::SetRotation(float pitch, float yaw, float roll)
{
    m_Rotation = { pitch, yaw, roll };
}
void Mesh::SetScale(float sx, float sy, float sz)
{
    m_Scale = { sx, sy, sz };
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void Mesh::UpdateModelUniformBuffer(void* modelDst) const
{
    glm::mat4 modelSrc = CalculateTransform();
    memcpy(modelDst, &modelSrc, sizeof(modelSrc));
}

void Mesh::UpdateDescriptorSets(VkDescriptorSet descriptorSet, VkSampler sampler) const
{
    // Images info
    std::vector<VkDescriptorImageInfo> imageInfos{ m_Textures.size() };
    for (size_t i{ 0 }; i < imageInfos.size(); ++i)
    {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = m_Textures[i].ImageView;
        imageInfos[i].sampler = sampler;
    }

    // The configuration of descriptor
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
    descriptorWrite.pImageInfo = imageInfos.data();

    // Update the configuration of the descriptor
    vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
}

void Mesh::CmdBindings(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet descriptorSet) const
{
    // Bind descriptor set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);

    // Bind vertex buffer
    std::vector<VkBuffer> vertexBuffers{ m_VertexIndexBuffer };
    std::vector<VkDeviceSize> offsets{ 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, m_VertexIndexBuffer, m_IndexOffset, VK_INDEX_TYPE_UINT32);

    // Bind drawing
    vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
}

void Mesh::CreateVertexIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
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

void Mesh::LoadModel(const char* filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices)
{
    // Load in the data (will triangulate by default)
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{}, err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath)) {
        throw std::runtime_error(warn + err);
    }

    // TODO: LoadModel make temporary glm::vec vectors for attrib.vertices and attrib.texcoords
    std::unordered_map<Vertex3D, uint32_t> uniqueVertices{};

    // Loop over all the OBJ shapes
    for (const tinyobj::shape_t& shape : shapes)
    {
        // TODO: LoadModel OBJ keep num_face_vertices in account when looping over faces
        // Loop over all the faces
        for (const tinyobj::index_t& index : shape.mesh.indices)
        {
            // Retrieve vertex data
            Vertex3D vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = { 1.0f, 1.0f, 1.0f };

            // Add vertex with index if it doesn't exist yet
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            // Add index data to the list
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

uint32_t Mesh::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // Available types of memory
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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

void Mesh::CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& buffer, GP2_VkDeviceMemory& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    // Create buffer resource
    buffer = std::move(GP2_VkBuffer{ device, size, usage, false });

    // Get buffer memory requirements
    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // Allocate buffer memory resource
    bufferMemory = std::move(GP2_VkDeviceMemory{ device, memRequirements.size, FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties) });

    // Associate memory with buffer
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

VkDeviceSize Mesh::CreateStagingBuffer(VkPhysicalDevice physicalDevice, VkDevice device, GP2_VkBuffer& stagingBuffer, GP2_VkDeviceMemory& stagingBufferMemory, const std::vector<VkDeviceSize>& sizes, const std::vector<const void*>& datas)
{
    // Exit early in case of wrong input values
    if (sizes.size() != datas.size() || sizes.empty()) return 0;

    // Calculate total buffer size
    VkDeviceSize bufferSize = std::accumulate(sizes.begin(), sizes.end(), static_cast<VkDeviceSize>(0));

    // Create staging buffer
    CreateBuffer(
        physicalDevice, device,
        stagingBuffer, stagingBufferMemory,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy datas to staging buffer
    void* dstData{};
    VkDeviceSize offset{ 0 };
    for (size_t i{ 0 }; i < datas.size(); ++i)
    {
        vkMapMemory(device, stagingBufferMemory, offset, sizes[i], 0, &dstData);
        memcpy(dstData, datas[i], sizes[i]);
        vkUnmapMemory(device, stagingBufferMemory);

        offset += sizes[i];
    }

    // Return the size of the staging buffer
    return bufferSize;
}

void Mesh::CopyBuffer(std::unique_ptr<GP2_SingleTimeCommand> commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    // Copy buffer command
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer->Get(), srcBuffer, dstBuffer, 1, &copyRegion);
}

