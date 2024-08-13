//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "PoolCommandBuffers.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
PoolCommandBuffers::PoolCommandBuffers(const VkDevice& device, uint32_t queueFamilyIndex, uint32_t count)
	: m_CommandPool{ device, queueFamilyIndex }
	, m_CommandBuffers{ count }
{
	// Allocation info
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
std::vector<VkCommandBuffer>& PoolCommandBuffers::Get()
{
	return m_CommandBuffers;
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

