#ifndef GP2VKT_GP2_VKPIPELINELAYOUT_H_
#define GP2VKT_GP2_VKPIPELINELAYOUT_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkPipelineLayout
class GP2_VkPipelineLayout final
{
public:
	// Constructors and Destructor
	GP2_VkPipelineLayout() = default;
	GP2_VkPipelineLayout(const VkDevice& device, const std::vector<VkDescriptorSetLayout>& setLayouts);
	~GP2_VkPipelineLayout();
	
	// Copy and Move semantics
	GP2_VkPipelineLayout(const GP2_VkPipelineLayout& other)					= delete;
	GP2_VkPipelineLayout& operator=(const GP2_VkPipelineLayout& other)		= delete;
	GP2_VkPipelineLayout(GP2_VkPipelineLayout&& other) noexcept				;
	GP2_VkPipelineLayout& operator=(GP2_VkPipelineLayout&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkPipelineLayout() const { return m_PipelineLayout; }
	explicit operator const VkPipelineLayout& () const { return m_PipelineLayout; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkPipelineLayout m_PipelineLayout{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif