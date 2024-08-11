//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkImage.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkImage::GP2_VkImage(const VkDevice& device, VkFormat format, VkExtent3D extent, VkImageTiling tiling, VkImageUsageFlags usage, bool isShared)
	: m_Device{ device }
	, m_Image{}
{
	// TODO: add checks to make sure the image is initialized correctly

	// Create info
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D; // Specifies coordinate system
	createInfo.format = format; // Should be the same format as the pixels in the buffer
	createInfo.extent = extent;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Only relevant for images used as attachments
	createInfo.tiling = tiling; // This can not be changed at a later time
	createInfo.usage = usage;
	createInfo.sharingMode = isShared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;; // Specifies sharing between queue families
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Either UNDEFINED or PREINITIALIZED

	// Create image
	if (vkCreateImage(m_Device, &createInfo, nullptr, &m_Image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image!");
}

GP2_VkImage::GP2_VkImage(GP2_VkImage&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Image{ other.m_Image }
{
	// Make other object invalid
	other.m_Image = nullptr;
}

GP2_VkImage& GP2_VkImage::operator=(GP2_VkImage&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyImage(m_Device, m_Image, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Image = other.m_Image;

		// Make other object invalid
		other.m_Image = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkImage::~GP2_VkImage()
{
	if (m_Device) vkDestroyImage(m_Device, m_Image, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkImage& GP2_VkImage::Get()
{
	return m_Image;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

