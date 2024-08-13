//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkBuffer.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkBuffer::GP2_VkBuffer(const VkDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, bool isShared)
	: m_Device{ device }
	, m_Buffer{}
{
	// Create info
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size; // Size of buffer in bytes
	createInfo.usage = usage; // Which purposes the data is going to be used for
	// Specify if the buffer is shared between queue families
	createInfo.sharingMode = isShared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

	// Create buffer
	if (vkCreateBuffer(m_Device, &createInfo, nullptr, &m_Buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer!");
}

GP2_VkBuffer::GP2_VkBuffer(GP2_VkBuffer&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Buffer{ other.m_Buffer }
{
	// Make other object invalid
	other.m_Buffer = nullptr;
}

GP2_VkBuffer& GP2_VkBuffer::operator=(GP2_VkBuffer&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyBuffer(m_Device, m_Buffer, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Buffer = other.m_Buffer;

		// Make other object invalid
		other.m_Buffer = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkBuffer::~GP2_VkBuffer()
{
	if (m_Device) vkDestroyBuffer(m_Device, m_Buffer, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

