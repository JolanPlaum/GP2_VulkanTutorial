#ifndef GP2VKT_GP2_VKDESCRIPTORSETLAYOUT_H_
#define GP2VKT_GP2_VKDESCRIPTORSETLAYOUT_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkDescriptorSetLayout
class GP2_VkDescriptorSetLayout final
{
public:
	// Constructors and Destructor
	GP2_VkDescriptorSetLayout() = default;
	GP2_VkDescriptorSetLayout(const VkDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~GP2_VkDescriptorSetLayout();
	
	// Copy and Move semantics
	GP2_VkDescriptorSetLayout(const GP2_VkDescriptorSetLayout& other)					= delete;
	GP2_VkDescriptorSetLayout& operator=(const GP2_VkDescriptorSetLayout& other)		= delete;
	GP2_VkDescriptorSetLayout(GP2_VkDescriptorSetLayout&& other) noexcept				;
	GP2_VkDescriptorSetLayout& operator=(GP2_VkDescriptorSetLayout&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkDescriptorSetLayout& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkDescriptorSetLayout m_DescriptorSetLayout{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif