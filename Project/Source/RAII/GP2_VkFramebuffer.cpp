//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkFramebuffer.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkFramebuffer::GP2_VkFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, const VkExtent2D& extent, uint32_t layerCount)
	: m_Device{ device }
	, m_Framebuffer{}
{
	// Create info
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.width = extent.width;
	createInfo.height = extent.height;
	createInfo.layers = layerCount;

	// Create frame buffer using specified data
	if (vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create frame buffer!");
}

GP2_VkFramebuffer::GP2_VkFramebuffer(GP2_VkFramebuffer&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Framebuffer{ other.m_Framebuffer }
{
	// Make other object invalid
	other.m_Framebuffer = nullptr;
}

GP2_VkFramebuffer& GP2_VkFramebuffer::operator=(GP2_VkFramebuffer&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Framebuffer = other.m_Framebuffer;

		// Make other object invalid
		other.m_Framebuffer = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkFramebuffer::~GP2_VkFramebuffer()
{
	vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkFramebuffer& GP2_VkFramebuffer::Get()
{
	return m_Framebuffer;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

