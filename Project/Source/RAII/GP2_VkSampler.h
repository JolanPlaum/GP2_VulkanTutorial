#ifndef GP2VKT_GP2_VKSAMPLER_H_
#define GP2VKT_GP2_VKSAMPLER_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkSampler
class GP2_VkSampler final
{
public:
	// Constructors and Destructor
	GP2_VkSampler() = default;
	GP2_VkSampler(const VkDevice& device,
		VkSamplerAddressMode addressMode,
		float maxAnisotropy,
		bool unnormalizedCoordinates = false);
	~GP2_VkSampler();
	
	// Copy and Move semantics
	GP2_VkSampler(const GP2_VkSampler& other)					= delete;
	GP2_VkSampler& operator=(const GP2_VkSampler& other)		= delete;
	GP2_VkSampler(GP2_VkSampler&& other) noexcept				;
	GP2_VkSampler& operator=(GP2_VkSampler&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkSampler& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkSampler m_Sampler{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif