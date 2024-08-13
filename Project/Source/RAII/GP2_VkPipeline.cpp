//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkPipeline.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkPipeline::GP2_VkPipeline(const VkDevice& device, const VkGraphicsPipelineCreateInfo& createInfo)
	: m_Device{ device }
	, m_Pipeline{}
{
	// Create graphics pipeline
	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline!");
}

GP2_VkPipeline::GP2_VkPipeline(GP2_VkPipeline&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Pipeline{ other.m_Pipeline }
{
	// Make other object invalid
	other.m_Pipeline = nullptr;
}

GP2_VkPipeline& GP2_VkPipeline::operator=(GP2_VkPipeline&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyPipeline(m_Device, m_Pipeline, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Pipeline = other.m_Pipeline;

		// Make other object invalid
		other.m_Pipeline = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkPipeline::~GP2_VkPipeline()
{
	if (m_Device) vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

