//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkDescriptorSetLayout.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkDescriptorSetLayout::GP2_VkDescriptorSetLayout(const VkDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	: m_Device{ device }
	, m_DescriptorSetLayout{}
{
	// Create info
	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	createInfo.pBindings = bindings.data();

	// Create descriptor set layout
	if (vkCreateDescriptorSetLayout(m_Device, &createInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

GP2_VkDescriptorSetLayout::GP2_VkDescriptorSetLayout(GP2_VkDescriptorSetLayout&& other) noexcept
	: m_Device{ other.m_Device }
	, m_DescriptorSetLayout{ other.m_DescriptorSetLayout }
{
	// Make other object invalid
	other.m_DescriptorSetLayout = nullptr;
}

GP2_VkDescriptorSetLayout& GP2_VkDescriptorSetLayout::operator=(GP2_VkDescriptorSetLayout&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_DescriptorSetLayout = other.m_DescriptorSetLayout;

		// Make other object invalid
		other.m_DescriptorSetLayout = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkDescriptorSetLayout::~GP2_VkDescriptorSetLayout()
{
	if (m_Device) vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkDescriptorSetLayout& GP2_VkDescriptorSetLayout::Get()
{
	return m_DescriptorSetLayout;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

