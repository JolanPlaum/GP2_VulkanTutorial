#ifndef GP2VKT_GP2_VKDEVICE_H_
#define GP2VKT_GP2_VKDEVICE_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkDevice
class GP2_VkDevice final
{
public:
	// Constructors and Destructor
	GP2_VkDevice(const VkPhysicalDevice& physicalDevice,
		const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos,
		const std::vector<const char*>& enabledLayers,
		const std::vector<const char*>& enabledExtensions,
		const VkPhysicalDeviceFeatures& deviceFeatures);
	~GP2_VkDevice();
	
	// Copy and Move semantics
	GP2_VkDevice(const GP2_VkDevice& other)					= delete;
	GP2_VkDevice& operator=(const GP2_VkDevice& other)		= delete;
	GP2_VkDevice(GP2_VkDevice&& other) noexcept				;
	GP2_VkDevice& operator=(GP2_VkDevice&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkDevice& Get();


private:
	// Member variables
	VkDevice m_Device;

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif