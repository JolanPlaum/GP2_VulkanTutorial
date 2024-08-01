#ifndef GP2VKT_GP2_VKDEVICEMEMORY_H_
#define GP2VKT_GP2_VKDEVICEMEMORY_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkDeviceMemory
class GP2_VkDeviceMemory final
{
public:
	// Constructors and Destructor
	GP2_VkDeviceMemory() = default;
	GP2_VkDeviceMemory(const VkDevice& device, VkDeviceSize allocationSize, uint32_t memoryTypeIndex);
	~GP2_VkDeviceMemory();
	
	// Copy and Move semantics
	GP2_VkDeviceMemory(const GP2_VkDeviceMemory& other)					= delete;
	GP2_VkDeviceMemory& operator=(const GP2_VkDeviceMemory& other)		= delete;
	GP2_VkDeviceMemory(GP2_VkDeviceMemory&& other) noexcept				;
	GP2_VkDeviceMemory& operator=(GP2_VkDeviceMemory&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkDeviceMemory& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkDeviceMemory m_Memory{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif