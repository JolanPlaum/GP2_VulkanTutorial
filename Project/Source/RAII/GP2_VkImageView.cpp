//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkImageView.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkImageView::GP2_VkImageView(const VkDevice& device, const VkImage& image, const VkFormat& format)
	: m_Device{ device }
	, m_ImageView{}
{
	// Create info
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;

	// Specify how the image data should be interpreted
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;

	// Specify color channel swizzling
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Describe the image purpose & which part to access
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	// Create image view using specified data
	if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_ImageView) != VK_SUCCESS)
		throw std::runtime_error("failed to create image view!");
}

GP2_VkImageView::GP2_VkImageView(GP2_VkImageView&& other) noexcept
	: m_Device{ other.m_Device }
	, m_ImageView{ other.m_ImageView }
{
	// Make other object invalid
	other.m_ImageView = nullptr;
}

GP2_VkImageView& GP2_VkImageView::operator=(GP2_VkImageView&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyImageView(m_Device, m_ImageView, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_ImageView = other.m_ImageView;

		// Make other object invalid
		other.m_ImageView = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkImageView::~GP2_VkImageView()
{
	if (m_Device) vkDestroyImageView(m_Device, m_ImageView, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

