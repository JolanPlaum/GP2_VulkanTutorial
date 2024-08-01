//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkRenderPass.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkRenderPass::GP2_VkRenderPass(const VkDevice& device, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies)
	: m_Device{ device }
	, m_RenderPass{}
{
	// Create the render pass using the attachments and subpasses
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();

	createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	createInfo.pSubpasses = subpasses.data();

	createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	createInfo.pDependencies = dependencies.data();

	// Create render pass
	if (vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		throw std::runtime_error("failed to create render pass!");
}

GP2_VkRenderPass::GP2_VkRenderPass(GP2_VkRenderPass&& other) noexcept
	: m_Device{ other.m_Device }
	, m_RenderPass{ other.m_RenderPass }
{
	// Make other object invalid
	other.m_RenderPass = nullptr;
}

GP2_VkRenderPass& GP2_VkRenderPass::operator=(GP2_VkRenderPass&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_RenderPass = other.m_RenderPass;

		// Make other object invalid
		other.m_RenderPass = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkRenderPass::~GP2_VkRenderPass()
{
	if (m_Device) vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkRenderPass& GP2_VkRenderPass::Get()
{
	return m_RenderPass;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

