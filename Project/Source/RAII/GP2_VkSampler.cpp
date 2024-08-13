//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkSampler.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkSampler::GP2_VkSampler(const VkDevice& device, VkSamplerAddressMode addressMode, float maxAnisotropy, bool unnormalizedCoordinates)
	: m_Device{ device }
	, m_Sampler{}
{
	// Create info
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR; // Specifies how to interpolate texels that are oversampled
	createInfo.minFilter = VK_FILTER_LINEAR; // Specifies how to interpolate texels that are undersampled

	createInfo.addressModeU = addressMode;
	createInfo.addressModeV = addressMode;
	createInfo.addressModeW = addressMode;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	// TODO: match anisotropyEnable with VkPhysicalDeviceFeatures::samplerAnisotropy in logical device
	createInfo.anisotropyEnable = static_cast<VkBool32>(maxAnisotropy >= 1.0f); // Disabling will lead to better performance
	createInfo.maxAnisotropy = maxAnisotropy; // A lower value results in better performance

	createInfo.compareEnable = VK_FALSE; // Mainly used for shadow maps
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = static_cast<VkBool32>(unnormalizedCoordinates); // Specifies if texel coordinates are within [0, 1] range or [0, texWidth/height] range


	// Create sampler
	if (vkCreateSampler(m_Device, &createInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create sampler!");
}

GP2_VkSampler::GP2_VkSampler(GP2_VkSampler&& other) noexcept
	: m_Device{ other.m_Device }
	, m_Sampler{ other.m_Sampler }
{
	// Make other object invalid
	other.m_Sampler = nullptr;
}

GP2_VkSampler& GP2_VkSampler::operator=(GP2_VkSampler&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Device) vkDestroySampler(m_Device, m_Sampler, nullptr);

		// Assign new data
		m_Device = other.m_Device;
		m_Sampler = other.m_Sampler;

		// Make other object invalid
		other.m_Sampler = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkSampler::~GP2_VkSampler()
{
	if (m_Device) vkDestroySampler(m_Device, m_Sampler, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

