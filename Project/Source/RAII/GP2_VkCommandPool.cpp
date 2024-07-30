//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkCommandPool.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkCommandPool::GP2_VkCommandPool(const VkDevice& device, uint32_t queueFamilyIndex)
	: m_Device{ device }
	, m_CommandPool{}
{
	// A pool can only allocate command buffers on a single type of queue
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	// Create command pool
	if (vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

GP2_VkCommandPool::GP2_VkCommandPool(GP2_VkCommandPool&& other) noexcept
	: m_Device{ other.m_Device }
	, m_CommandPool{ other.m_CommandPool }
{
	// Make other object invalid
	other.m_CommandPool = nullptr;
}

GP2_VkCommandPool& GP2_VkCommandPool::operator=(GP2_VkCommandPool&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_CommandPool = other.m_CommandPool;

		// Make other object invalid
		other.m_CommandPool = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkCommandPool::~GP2_VkCommandPool()
{
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkCommandPool& GP2_VkCommandPool::Get()
{
	return m_CommandPool;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

