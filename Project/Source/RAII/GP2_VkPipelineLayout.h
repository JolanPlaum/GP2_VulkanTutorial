#ifndef GP2VKT_GP2_VKPIPELINELAYOUT_H_
#define GP2VKT_GP2_VKPIPELINELAYOUT_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkPipelineLayout
class GP2_VkPipelineLayout final
{
public:
	// Constructors and Destructor
	GP2_VkPipelineLayout() = default;
	GP2_VkPipelineLayout(const VkDevice& device);
	~GP2_VkPipelineLayout();
	
	// Copy and Move semantics
	GP2_VkPipelineLayout(const GP2_VkPipelineLayout& other)					= delete;
	GP2_VkPipelineLayout& operator=(const GP2_VkPipelineLayout& other)		= delete;
	GP2_VkPipelineLayout(GP2_VkPipelineLayout&& other) noexcept				;
	GP2_VkPipelineLayout& operator=(GP2_VkPipelineLayout&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkPipelineLayout& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkPipelineLayout m_PipelineLayout{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif