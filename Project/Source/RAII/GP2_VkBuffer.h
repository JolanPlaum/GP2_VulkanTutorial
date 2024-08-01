#ifndef GP2VKT_GP2_VKBUFFER_H_
#define GP2VKT_GP2_VKBUFFER_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkBuffer
class GP2_VkBuffer final
{
public:
	// Constructors and Destructor
	GP2_VkBuffer(const VkDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, bool isShared = false);
	~GP2_VkBuffer();
	
	// Copy and Move semantics
	GP2_VkBuffer(const GP2_VkBuffer& other)					= delete;
	GP2_VkBuffer& operator=(const GP2_VkBuffer& other)		= delete;
	GP2_VkBuffer(GP2_VkBuffer&& other) noexcept				;
	GP2_VkBuffer& operator=(GP2_VkBuffer&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkBuffer& Get();


private:
	// Member variables
	VkDevice m_Device;
	VkBuffer m_Buffer;

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif