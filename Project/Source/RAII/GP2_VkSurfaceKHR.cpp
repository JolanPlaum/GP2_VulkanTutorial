//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkSurfaceKHR.h"
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkSurfaceKHR::GP2_VkSurfaceKHR(const VkInstance& instance, GLFWwindow* pWindow)
	: m_Instance{ instance }
	, m_Surface{}
{
	// Create surface
	if (glfwCreateWindowSurface(m_Instance, pWindow, nullptr, &m_Surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}

GP2_VkSurfaceKHR::GP2_VkSurfaceKHR(GP2_VkSurfaceKHR&& other) noexcept
	: m_Instance{ other.m_Instance }
	, m_Surface{ other.m_Surface }
{
	// Make other object invalid
	other.m_Surface = nullptr;
}

GP2_VkSurfaceKHR& GP2_VkSurfaceKHR::operator=(GP2_VkSurfaceKHR&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Instance) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

		// Assign new data
		m_Instance = other.m_Instance;
		m_Surface = other.m_Surface;

		// Make other object invalid
		other.m_Surface = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkSurfaceKHR::~GP2_VkSurfaceKHR()
{
	if (m_Instance) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkSurfaceKHR& GP2_VkSurfaceKHR::Get()
{
	return m_Surface;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

