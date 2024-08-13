//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkDescriptorPool.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkDescriptorPool::GP2_VkDescriptorPool(const VkDevice& device, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes)
	: m_Device{ device }
	, m_VkDescriptorPool{}
{
	// Create info
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = maxSets;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();

	// Create descriptor pool
	if (vkCreateDescriptorPool(m_Device, &createInfo, nullptr, &m_VkDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}

GP2_VkDescriptorPool::GP2_VkDescriptorPool(GP2_VkDescriptorPool&& other) noexcept
	: m_Device{ other.m_Device }
	, m_VkDescriptorPool{ other.m_VkDescriptorPool }
{
	// Make other object invalid
	other.m_VkDescriptorPool = nullptr;
}

GP2_VkDescriptorPool& GP2_VkDescriptorPool::operator=(GP2_VkDescriptorPool&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyDescriptorPool(m_Device, m_VkDescriptorPool, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_VkDescriptorPool = other.m_VkDescriptorPool;

		// Make other object invalid
		other.m_VkDescriptorPool = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkDescriptorPool::~GP2_VkDescriptorPool()
{
	if (m_Device) vkDestroyDescriptorPool(m_Device, m_VkDescriptorPool, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

