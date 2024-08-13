//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "PoolDescriptorSets.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
PoolDescriptorSets::PoolDescriptorSets(const VkDevice& device, const std::vector<VkDescriptorPoolSize>& poolSizes, const VkDescriptorSetLayout& layout, uint32_t count)
	: m_DescriptorPool{ device, count, poolSizes }
	, m_DescriptorSets{ count }
{
	// Each descriptor pool will only have a single layout allocating from it to prevent fragmentation
	// This is not the best solution but works for the scale of this project
	std::vector<VkDescriptorSetLayout> layouts{ count, layout };

	// Allocate info
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	// Allocate descriptor sets
	if (vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
std::vector<VkDescriptorSet>& PoolDescriptorSets::Get()
{
	return m_DescriptorSets;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

