//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkPipelineLayout.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkPipelineLayout::GP2_VkPipelineLayout(const VkDevice& device, const std::vector<VkDescriptorSetLayout>& setLayouts)
	: m_Device{ device }
	, m_PipelineLayout{}
{
	// Specify uniform values used in shaders (global values similar to dynamic state variables)
	// Commonly used to pass the transformation matrix to the vertex shader
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	createInfo.pSetLayouts = setLayouts.data();
	createInfo.pushConstantRangeCount = 0;
	createInfo.pPushConstantRanges = nullptr;

	// Create pipeline layout
	if (vkCreatePipelineLayout(m_Device, &createInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}

GP2_VkPipelineLayout::GP2_VkPipelineLayout(GP2_VkPipelineLayout&& other) noexcept
	: m_Device{ other.m_Device }
	, m_PipelineLayout{ other.m_PipelineLayout }
{
	// Make other object invalid
	other.m_PipelineLayout = nullptr;
}

GP2_VkPipelineLayout& GP2_VkPipelineLayout::operator=(GP2_VkPipelineLayout&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_PipelineLayout = other.m_PipelineLayout;

		// Make other object invalid
		other.m_PipelineLayout = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkPipelineLayout::~GP2_VkPipelineLayout()
{
	if (m_Device) vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

