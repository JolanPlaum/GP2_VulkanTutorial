#ifndef GP2VKT_TEXTURE_H_
#define GP2VKT_TEXTURE_H_
// Includes
#include "RAII/GP2_VkImage.h"
#include "RAII/GP2_VkDeviceMemory.h"
#include "RAII/GP2_VkImageView.h"

// Class Forward Declarations


// TODO: put functionality inside of Texture class instead of treating it like a data class
// The Texture class wraps all the variables needed for a texture (Image, ImageMemory & ImageView)
class Texture final
{
public:
	// Constructors and Destructor
	/*explicit Texture(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkExtent2D extent,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		const char* filePath);*/
	explicit Texture() = default;
	~Texture() = default;
	
	// Copy and Move semantics
	Texture(const Texture& other)					= delete;
	Texture& operator=(const Texture& other)		= delete;
	Texture(Texture&& other) noexcept				= default;
	Texture& operator=(Texture&& other) noexcept	= default;

	//---------------------------
	// Public Member Functions
	//---------------------------
	GP2_VkImage Image{};
	GP2_VkDeviceMemory ImageMemory{};
	GP2_VkImageView ImageView{};


private:
	// Member variables
	/*GP2_VkImage m_Image{};
	GP2_VkDeviceMemory m_ImageMemory{};
	GP2_VkImageView m_ImageView{};*/

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif