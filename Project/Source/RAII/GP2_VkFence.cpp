//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkFence.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkFence::GP2_VkFence(const VkDevice& device, bool signaled)
	: m_Device{ device }
	, m_Fence{}
{
	// Doesn't have any required fields in the current version of API
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT * signaled;

	// Create fence
	if (vkCreateFence(m_Device, &createInfo, nullptr, &m_Fence) != VK_SUCCESS)
		throw std::runtime_error("failed to create fence!");
}

GP2_VkFence::GP2_VkFence(GP2_VkFence&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Fence{ other.m_Fence }
{
	// Make other object invalid
	other.m_Fence = nullptr;
}

GP2_VkFence& GP2_VkFence::operator=(GP2_VkFence&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyFence(m_Device, m_Fence, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Fence = other.m_Fence;

		// Make other object invalid
		other.m_Fence = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkFence::~GP2_VkFence()
{
	if (m_Device) vkDestroyFence(m_Device, m_Fence, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkFence& GP2_VkFence::Get()
{
	return m_Fence;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

