#ifndef GP2VKT_GP2_VKCOMMANDPOOL_H_
#define GP2VKT_GP2_VKCOMMANDPOOL_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkCommandPool
class GP2_VkCommandPool final
{
public:
	// Constructors and Destructor
	GP2_VkCommandPool() = default;
	GP2_VkCommandPool(const VkDevice& device, uint32_t queueFamilyIndex);
	~GP2_VkCommandPool();
	
	// Copy and Move semantics
	GP2_VkCommandPool(const GP2_VkCommandPool& other)					= delete;
	GP2_VkCommandPool& operator=(const GP2_VkCommandPool& other)		= delete;
	GP2_VkCommandPool(GP2_VkCommandPool&& other) noexcept				;
	GP2_VkCommandPool& operator=(GP2_VkCommandPool&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkCommandPool() const { return m_CommandPool; }
	explicit operator const VkCommandPool& () const { return m_CommandPool; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkCommandPool m_CommandPool{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif