#ifndef GP2VKT_GP2_VKDESCRIPTORPOOL_H_
#define GP2VKT_GP2_VKDESCRIPTORPOOL_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkDescriptorPool
class GP2_VkDescriptorPool final
{
public:
	// Constructors and Destructor
	GP2_VkDescriptorPool() = default;
	GP2_VkDescriptorPool(const VkDevice& device, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes);
	~GP2_VkDescriptorPool();
	
	// Copy and Move semantics
	GP2_VkDescriptorPool(const GP2_VkDescriptorPool& other)					= delete;
	GP2_VkDescriptorPool& operator=(const GP2_VkDescriptorPool& other)		= delete;
	GP2_VkDescriptorPool(GP2_VkDescriptorPool&& other) noexcept				;
	GP2_VkDescriptorPool& operator=(GP2_VkDescriptorPool&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkDescriptorPool& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkDescriptorPool m_VkDescriptorPool{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif