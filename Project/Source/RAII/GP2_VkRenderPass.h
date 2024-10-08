#ifndef GP2VKT_GP2_VKRENDERPASS_H_
#define GP2VKT_GP2_VKRENDERPASS_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations


// RAII wrapper for VkRenderPass
class GP2_VkRenderPass final
{
public:
	// Constructors and Destructor
	GP2_VkRenderPass() = default;
	GP2_VkRenderPass(const VkDevice& device,
		const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& dependencies);
	~GP2_VkRenderPass();
	
	// Copy and Move semantics
	GP2_VkRenderPass(const GP2_VkRenderPass& other)					= delete;
	GP2_VkRenderPass& operator=(const GP2_VkRenderPass& other)		= delete;
	GP2_VkRenderPass(GP2_VkRenderPass&& other) noexcept				;
	GP2_VkRenderPass& operator=(GP2_VkRenderPass&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkRenderPass() const { return m_RenderPass; }
	explicit operator const VkRenderPass& () const { return m_RenderPass; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkRenderPass m_RenderPass{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif