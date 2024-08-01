#ifndef GP2VKT_GP2_VKIMAGEVIEW_H_
#define GP2VKT_GP2_VKIMAGEVIEW_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkImageView
class GP2_VkImageView final
{
public:
	// Constructors and Destructor
	GP2_VkImageView() = default;
	GP2_VkImageView(const VkDevice& device, const VkImage& image, const VkFormat& format);
	~GP2_VkImageView();
	
	// Copy and Move semantics
	GP2_VkImageView(const GP2_VkImageView& other)					= delete;
	GP2_VkImageView& operator=(const GP2_VkImageView& other)		= delete;
	GP2_VkImageView(GP2_VkImageView&& other) noexcept				;
	GP2_VkImageView& operator=(GP2_VkImageView&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkImageView& Get();


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkImageView m_ImageView{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif