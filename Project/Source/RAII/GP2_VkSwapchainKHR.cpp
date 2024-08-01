//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkSwapchainKHR.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkSwapchainKHR::GP2_VkSwapchainKHR(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo)
	: m_Device{ device }
	, m_SwapchainKHR{}
{
	// Create swap chain
	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapchainKHR) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain!");
}

GP2_VkSwapchainKHR::GP2_VkSwapchainKHR(GP2_VkSwapchainKHR&& other) noexcept
	: m_Device{ other.m_Device }
	, m_SwapchainKHR{ other.m_SwapchainKHR }
{
	// Make other object invalid
	other.m_SwapchainKHR = nullptr;
}

GP2_VkSwapchainKHR& GP2_VkSwapchainKHR::operator=(GP2_VkSwapchainKHR&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroySwapchainKHR(m_Device, m_SwapchainKHR, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_SwapchainKHR = other.m_SwapchainKHR;

		// Make other object invalid
		other.m_SwapchainKHR = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkSwapchainKHR::~GP2_VkSwapchainKHR()
{
	if (m_Device) vkDestroySwapchainKHR(m_Device, m_SwapchainKHR, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkSwapchainKHR& GP2_VkSwapchainKHR::Get()
{
	return m_SwapchainKHR;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

