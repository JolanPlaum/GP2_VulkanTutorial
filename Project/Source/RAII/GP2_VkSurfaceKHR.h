#ifndef GP2VKT_GP2_VKSURFACEKHR_H_
#define GP2VKT_GP2_VKSURFACEKHR_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations
struct GLFWwindow;


// RAII wrapper for VkSurfaceKHR
class GP2_VkSurfaceKHR final
{
public:
	// Constructors and Destructor
	GP2_VkSurfaceKHR() = default;
	GP2_VkSurfaceKHR(const VkInstance& instance, GLFWwindow* pWindow);
	~GP2_VkSurfaceKHR();
	
	// Copy and Move semantics
	GP2_VkSurfaceKHR(const GP2_VkSurfaceKHR& other)					= delete;
	GP2_VkSurfaceKHR& operator=(const GP2_VkSurfaceKHR& other)		= delete;
	GP2_VkSurfaceKHR(GP2_VkSurfaceKHR&& other) noexcept				;
	GP2_VkSurfaceKHR& operator=(GP2_VkSurfaceKHR&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkSurfaceKHR& Get();


private:
	// Member variables
	VkInstance m_Instance{ nullptr };
	VkSurfaceKHR m_Surface{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif