#ifndef GP2VKT_GP2_VKFENCE_H_
#define GP2VKT_GP2_VKFENCE_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkFence
class GP2_VkFence final
{
public:
	// Constructors and Destructor
	GP2_VkFence() = default;
	GP2_VkFence(const VkDevice& device, bool signaled = false);
	~GP2_VkFence();
	
	// Copy and Move semantics
	GP2_VkFence(const GP2_VkFence& other)					= delete;
	GP2_VkFence& operator=(const GP2_VkFence& other)		= delete;
	GP2_VkFence(GP2_VkFence&& other) noexcept				;
	GP2_VkFence& operator=(GP2_VkFence&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkFence& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkFence m_Fence{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif