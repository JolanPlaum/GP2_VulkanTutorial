#ifndef GP2VKT_GP2_VKPIPELINE_H_
#define GP2VKT_GP2_VKPIPELINE_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkPipeline
class GP2_VkPipeline final
{
public:
	// Constructors and Destructor
	GP2_VkPipeline() = default;
	GP2_VkPipeline(const VkDevice& device, const VkGraphicsPipelineCreateInfo& createInfo);
	~GP2_VkPipeline();
	
	// Copy and Move semantics
	GP2_VkPipeline(const GP2_VkPipeline& other)					= delete;
	GP2_VkPipeline& operator=(const GP2_VkPipeline& other)		= delete;
	GP2_VkPipeline(GP2_VkPipeline&& other) noexcept				;
	GP2_VkPipeline& operator=(GP2_VkPipeline&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkPipeline() const { return m_Pipeline; }
	explicit operator const VkPipeline& () const { return m_Pipeline; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkPipeline m_Pipeline{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif