#ifndef GP2VKT_GP2_VKIMAGE_H_
#define GP2VKT_GP2_VKIMAGE_H_
// Includes
#include <vulkan/vulkan_core.h>

// Class Forward Declarations


// RAII wrapper for VkImage
class GP2_VkImage final
{
public:
	// Constructors and Destructor
	GP2_VkImage() = default;
	GP2_VkImage(const VkDevice& device,
		VkFormat format,
		VkExtent3D extent,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		bool isShared);
	~GP2_VkImage();
	
	// Copy and Move semantics
	GP2_VkImage(const GP2_VkImage& other)					= delete;
	GP2_VkImage& operator=(const GP2_VkImage& other)		= delete;
	GP2_VkImage(GP2_VkImage&& other) noexcept				;
	GP2_VkImage& operator=(GP2_VkImage&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkImage() const { return m_Image; }
	explicit operator const VkImage& () const { return m_Image; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkImage m_Image{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif