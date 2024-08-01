#ifndef GP2VKT_GP2_VKINSTANCE_H_
#define GP2VKT_GP2_VKINSTANCE_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkInstance
class GP2_VkInstance final
{
public:
	// Constructors and Destructor
	GP2_VkInstance() = default;
	GP2_VkInstance(const VkApplicationInfo& appInfo, const std::vector<const char*>& extensions,
		bool enableLayers = false, const std::vector<const char*>& layers = {}, VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo = nullptr);
	~GP2_VkInstance();
	
	// Copy and Move semantics
	GP2_VkInstance(const GP2_VkInstance& other)					= delete;
	GP2_VkInstance& operator=(const GP2_VkInstance& other)		= delete;
	GP2_VkInstance(GP2_VkInstance&& other) noexcept				;
	GP2_VkInstance& operator=(GP2_VkInstance&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkInstance& Get();


private:
	// Member variables
	VkInstance m_Instance{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif