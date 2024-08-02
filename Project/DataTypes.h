#ifndef GP2VKT_DATATYPES_H_
#define GP2VKT_DATATYPES_H_
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
	glm::vec2 pos{ 0.f, 0.f };
	glm::vec3 color{ 0.f, 0.f, 0.f };

    static constexpr VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // Index of the binding in the array of bindings
        bindingDescription.stride = sizeof(Vertex); // Specifies number of bytes between entries
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after vertex/intance
        return bindingDescription;
    }

    static constexpr std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{}; // One for each member variable

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
#endif