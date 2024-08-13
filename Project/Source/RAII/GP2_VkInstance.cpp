//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkInstance.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkInstance::GP2_VkInstance(const VkApplicationInfo& appInfo, const std::vector<const char*>& extensions, bool enableLayers, const std::vector<const char*>& layers, VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo)
	: m_Instance{}
{
	// Create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableLayers) {
		// Layers info
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();

		// Debug info
		createInfo.pNext = pDebugInfo;
	}
	else {
		// Layers info
		createInfo.enabledLayerCount = 0;

		// Debug info
		createInfo.pNext = nullptr;
	}

	// Create instance using specified data
	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
}

GP2_VkInstance::GP2_VkInstance(GP2_VkInstance&& other) noexcept
	: m_Instance{ other.m_Instance }
{
	// Make other object invalid
	other.m_Instance = nullptr;
}

GP2_VkInstance& GP2_VkInstance::operator=(GP2_VkInstance&& other) noexcept
{
	// Exit early if same object
	if (this != &other)
	{
		// Destroy previously owned resource
		if (m_Instance) vkDestroyInstance(m_Instance, nullptr);

		// Assign new data
		m_Instance = other.m_Instance;

		// Make other object invalid
		other.m_Instance = nullptr;
	}
	return *this;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkInstance::~GP2_VkInstance()
{
	if (m_Instance) vkDestroyInstance(m_Instance, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

