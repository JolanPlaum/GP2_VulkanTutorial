#ifndef GP2VKT_DATATYPES_H_
#define GP2VKT_DATATYPES_H_
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <array>

struct Vertex2D
{
	glm::vec2 pos{ 0.f, 0.f };
	glm::vec3 color{ 0.f, 0.f, 0.f };
    glm::vec2 texCoord{ 0.f, 0.f };

    bool operator==(const Vertex2D&) const = default;

    static constexpr VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // Index of the binding in the array of bindings
        bindingDescription.stride = sizeof(Vertex2D); // Specifies number of bytes between entries
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after vertex/intance
        return bindingDescription;
    }
    static constexpr std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{}; // One for each member variable

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex2D, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex2D, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex2D, texCoord);

        return attributeDescriptions;
    }
};
template<> struct std::hash<Vertex2D>
{
   size_t operator()(Vertex2D const& vertex) const {
       return ((std::hash<glm::vec2>()(vertex.pos)
           ^ (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1)
           ^ (std::hash<glm::vec2>()(vertex.texCoord) << 1);
   }
};

struct Vertex3D
{
	glm::vec3 pos{ 0.f, 0.f, 0.f };
	glm::vec3 color{ 0.f, 0.f, 0.f };
    glm::vec2 texCoord{ 0.f, 0.f };

    bool operator==(const Vertex3D&) const = default;

    static constexpr VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // Index of the binding in the array of bindings
        bindingDescription.stride = sizeof(Vertex3D); // Specifies number of bytes between entries
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after vertex/intance
        return bindingDescription;
    }
    static constexpr std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{}; // One for each member variable

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex3D, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex3D, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex3D, texCoord);

        return attributeDescriptions;
    }
};
template<> struct std::hash<Vertex3D>
{
    size_t operator()(Vertex3D const& vertex) const {
        return ((std::hash<glm::vec3>()(vertex.pos) ^
            (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            (std::hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

struct VertexPBR
{
	glm::vec3 pos{ 0.f, 0.f, 0.f };
	glm::vec3 normal{ 0.f, 0.f, 0.f };
	glm::vec3 tangent{ 0.f, 0.f, 0.f };
	glm::vec2 texCoord{ 0.f, 0.f };

    bool operator==(const VertexPBR&) const = default;

    static constexpr VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // Index of the binding in the array of bindings
        bindingDescription.stride = sizeof(VertexPBR); // Specifies number of bytes between entries
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry after vertex/intance
        return bindingDescription;
    }
    static constexpr std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{}; // One for each member variable

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexPBR, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexPBR, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexPBR, tangent);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(VertexPBR, texCoord);

        return attributeDescriptions;
    }
};
template<> struct std::hash<VertexPBR>
{
    size_t operator()(VertexPBR const& vertex) const {
        return (( (( std::hash<glm::vec3>()(vertex.pos) ^
            (std::hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
            (std::hash<glm::vec3>()(vertex.tangent) << 1)) >> 1) ^
            (std::hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};
#endif