//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_GLFWwindow.h"
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_GLFWwindow::GP2_GLFWwindow(int width, int height, const char* title)
	: m_pWindow{}
{
	// Create GLFW window
	if (m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr); m_pWindow == nullptr)
		throw std::runtime_error("failed to create GLFW window!");
}

GP2_GLFWwindow::GP2_GLFWwindow(GP2_GLFWwindow&& other) noexcept
	: m_pWindow{ other.m_pWindow }
{
	// Make other object invalid
	other.m_pWindow = nullptr;
}

GP2_GLFWwindow& GP2_GLFWwindow::operator=(GP2_GLFWwindow&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_pWindow) glfwDestroyWindow(m_pWindow);

		// Assign new data
		m_pWindow = other.m_pWindow;

		// Make other object invalid
		other.m_pWindow = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_GLFWwindow::~GP2_GLFWwindow()
{
	if (m_pWindow) glfwDestroyWindow(m_pWindow);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

