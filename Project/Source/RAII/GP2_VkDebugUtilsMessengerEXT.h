#ifndef GP2VKT_GP2_VKDEBUGUTILSMESSENGEREXT_H_
#define GP2VKT_GP2_VKDEBUGUTILSMESSENGEREXT_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkDebugUtilsMessengerEXT
class GP2_VkDebugUtilsMessengerEXT final
{
public:
	// Constructors and Destructor
	GP2_VkDebugUtilsMessengerEXT() = default;
	GP2_VkDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	~GP2_VkDebugUtilsMessengerEXT();
	
	// Copy and Move semantics
	GP2_VkDebugUtilsMessengerEXT(const GP2_VkDebugUtilsMessengerEXT& other)					= delete;
	GP2_VkDebugUtilsMessengerEXT& operator=(const GP2_VkDebugUtilsMessengerEXT& other)		= delete;
	GP2_VkDebugUtilsMessengerEXT(GP2_VkDebugUtilsMessengerEXT&& other) noexcept				;
	GP2_VkDebugUtilsMessengerEXT& operator=(GP2_VkDebugUtilsMessengerEXT&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkDebugUtilsMessengerEXT() const { return m_DebugMessenger; }
	explicit operator const VkDebugUtilsMessengerEXT& () const { return m_DebugMessenger; }

	static VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);


private:
	// Member variables
	VkInstance m_Instance{ nullptr };
	VkDebugUtilsMessengerEXT m_DebugMessenger{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif