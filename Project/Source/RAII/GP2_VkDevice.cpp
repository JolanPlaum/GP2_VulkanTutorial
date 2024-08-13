//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkDevice.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkDevice::GP2_VkDevice(const VkPhysicalDevice& physicalDevice, const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const std::vector<const char*>& enabledLayers, const std::vector<const char*>& enabledExtensions, const VkPhysicalDeviceFeatures& deviceFeatures)
	: m_Device{}
{
	// Create info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	if (enabledLayers.empty() == false) {
		// These are ignored by up-to-date implementations
		// Good idea to set them anyway for compatibility with older versions
		createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
		createInfo.ppEnabledLayerNames = enabledLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Create device using specified data
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
		throw std::runtime_error("failed to create device!");
}

GP2_VkDevice::GP2_VkDevice(GP2_VkDevice&& other) noexcept
	: m_Device{ other.m_Device }
{
	// Make other object invalid
	other.m_Device = nullptr;
}

GP2_VkDevice& GP2_VkDevice::operator=(GP2_VkDevice&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyDevice(m_Device, nullptr);

		// Assign new data
		m_Device = other.m_Device;

		// Make other object invalid
		other.m_Device = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkDevice::~GP2_VkDevice()
{
	if (m_Device) vkDestroyDevice(m_Device, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

