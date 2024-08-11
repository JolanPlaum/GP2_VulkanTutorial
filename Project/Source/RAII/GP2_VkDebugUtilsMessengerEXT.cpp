//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkDebugUtilsMessengerEXT.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkDebugUtilsMessengerEXT::GP2_VkDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	: m_Instance{ instance }
	, m_DebugMessenger{}
{
	// Create debug messenger
	if (vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to create debug messenger!");
}

GP2_VkDebugUtilsMessengerEXT::GP2_VkDebugUtilsMessengerEXT(GP2_VkDebugUtilsMessengerEXT&& other) noexcept
	: m_Instance{ other.m_Instance }
	, m_DebugMessenger{ other.m_DebugMessenger }
{
	// Make other object invalid
	other.m_DebugMessenger = nullptr;
}

GP2_VkDebugUtilsMessengerEXT& GP2_VkDebugUtilsMessengerEXT::operator=(GP2_VkDebugUtilsMessengerEXT&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Instance) vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

		// Assign new data
		m_Instance = other.m_Instance;
		m_DebugMessenger = other.m_DebugMessenger;

		// Make other object invalid
		other.m_DebugMessenger = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkDebugUtilsMessengerEXT::~GP2_VkDebugUtilsMessengerEXT()
{
	if (m_Instance) vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkDebugUtilsMessengerEXT& GP2_VkDebugUtilsMessengerEXT::Get()
{
	return m_DebugMessenger;
}

VkResult GP2_VkDebugUtilsMessengerEXT::vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	// Error if the function couldn't be loaded
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void GP2_VkDebugUtilsMessengerEXT::vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

