//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkSemaphore.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkSemaphore::GP2_VkSemaphore(const VkDevice& device)
	: m_Device{ device }
	, m_Semaphore{}
{
	// Doesn't have any required fields in the current version of API
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create semaphore
	if (vkCreateSemaphore(m_Device, &createInfo, nullptr, &m_Semaphore) != VK_SUCCESS)
		throw std::runtime_error("failed to create semaphore!");
}

GP2_VkSemaphore::GP2_VkSemaphore(GP2_VkSemaphore&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Semaphore{ other.m_Semaphore }
{
	// Make other object invalid
	other.m_Semaphore = nullptr;
}

GP2_VkSemaphore& GP2_VkSemaphore::operator=(GP2_VkSemaphore&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		vkDestroySemaphore(m_Device, m_Semaphore, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Semaphore = other.m_Semaphore;

		// Make other object invalid
		other.m_Semaphore = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkSemaphore::~GP2_VkSemaphore()
{
	vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkSemaphore& GP2_VkSemaphore::Get()
{
	return m_Semaphore;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

