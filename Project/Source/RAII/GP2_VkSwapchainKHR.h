#ifndef GP2VKT_GP2_VKSWAPCHAINKHR_H_
#define GP2VKT_GP2_VKSWAPCHAINKHR_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkSwapchainKHR
class GP2_VkSwapchainKHR final
{
public:
	// Constructors and Destructor
	GP2_VkSwapchainKHR() = default;
	GP2_VkSwapchainKHR(const VkDevice& device, const VkSwapchainCreateInfoKHR& createInfo);
	~GP2_VkSwapchainKHR();
	
	// Copy and Move semantics
	GP2_VkSwapchainKHR(const GP2_VkSwapchainKHR& other)					= delete;
	GP2_VkSwapchainKHR& operator=(const GP2_VkSwapchainKHR& other)		= delete;
	GP2_VkSwapchainKHR(GP2_VkSwapchainKHR&& other) noexcept				;
	GP2_VkSwapchainKHR& operator=(GP2_VkSwapchainKHR&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkSwapchainKHR() const { return m_SwapchainKHR; }
	explicit operator const VkSwapchainKHR& () const { return m_SwapchainKHR; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkSwapchainKHR m_SwapchainKHR{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif