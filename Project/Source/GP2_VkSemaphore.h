#ifndef GP2VKT_GP2_VKSEMAPHORE_H_
#define GP2VKT_GP2_VKSEMAPHORE_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkSemaphore
class GP2_VkSemaphore final
{
public:
	// Constructors and Destructor
	GP2_VkSemaphore(const VkDevice& device);
	~GP2_VkSemaphore();
	
	// Copy and Move semantics
	GP2_VkSemaphore(const GP2_VkSemaphore& other)					= delete;
	GP2_VkSemaphore& operator=(const GP2_VkSemaphore& other)		= delete;
	GP2_VkSemaphore(GP2_VkSemaphore&& other) noexcept				;
	GP2_VkSemaphore& operator=(GP2_VkSemaphore&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkSemaphore& Get();


private:
	// Member variables
	VkDevice m_Device;
	VkSemaphore m_Semaphore;

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif