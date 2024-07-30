#ifndef GP2VKT_GP2_VKFRAMEBUFFER_H_
#define GP2VKT_GP2_VKFRAMEBUFFER_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkFramebuffer
class GP2_VkFramebuffer final
{
public:
	// Constructors and Destructor
	GP2_VkFramebuffer(const VkDevice& device,
		const VkRenderPass& renderPass,
		const std::vector<VkImageView>& attachments,
		const VkExtent2D& extent,
		uint32_t layerCount = 1);
	~GP2_VkFramebuffer();
	
	// Copy and Move semantics
	GP2_VkFramebuffer(const GP2_VkFramebuffer& other)					= delete;
	GP2_VkFramebuffer& operator=(const GP2_VkFramebuffer& other)		= delete;
	GP2_VkFramebuffer(GP2_VkFramebuffer&& other) noexcept				;
	GP2_VkFramebuffer& operator=(GP2_VkFramebuffer&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkFramebuffer& Get();


private:
	// Member variables
	VkDevice m_Device;
	VkFramebuffer m_Framebuffer;

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif