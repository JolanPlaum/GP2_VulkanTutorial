//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkDeviceMemory.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkDeviceMemory::GP2_VkDeviceMemory(const VkDevice& device, VkDeviceSize allocationSize, uint32_t memoryTypeIndex)
	: m_Device{ device }
	, m_Memory{}
{
	// Create info
	VkMemoryAllocateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	createInfo.allocationSize = allocationSize;
	createInfo.memoryTypeIndex = memoryTypeIndex;

	// Create memory using specified data
	if (vkAllocateMemory(m_Device, &createInfo, nullptr, &m_Memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory!");
}

GP2_VkDeviceMemory::GP2_VkDeviceMemory(GP2_VkDeviceMemory&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Memory{ other.m_Memory }
{
	// Make other object invalid
	other.m_Memory = nullptr;
}

GP2_VkDeviceMemory& GP2_VkDeviceMemory::operator=(GP2_VkDeviceMemory&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkFreeMemory(m_Device, m_Memory, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Memory = other.m_Memory;

		// Make other object invalid
		other.m_Memory = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkDeviceMemory::~GP2_VkDeviceMemory()
{
	if (m_Device) vkFreeMemory(m_Device, m_Memory, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkDeviceMemory& GP2_VkDeviceMemory::Get()
{
	return m_Memory;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

