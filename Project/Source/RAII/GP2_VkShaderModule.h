#ifndef GP2VKT_GP2_VKSHADERMODULE_H_
#define GP2VKT_GP2_VKSHADERMODULE_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <string>

// Class Forward Declarations


// RAII wrapper for VkShaderModule 
class GP2_VkShaderModule final
{
public:
	// Constructors and Destructor
	GP2_VkShaderModule() = default;
	GP2_VkShaderModule(const VkDevice& device, const std::string& path);
	~GP2_VkShaderModule();
	
	// Copy and Move semantics
	GP2_VkShaderModule(const GP2_VkShaderModule& other)					= delete;
	GP2_VkShaderModule& operator=(const GP2_VkShaderModule& other)		= delete;
	GP2_VkShaderModule(GP2_VkShaderModule&& other) noexcept				;
	GP2_VkShaderModule& operator=(GP2_VkShaderModule&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	operator VkShaderModule() const { return m_ShaderModule; }
	explicit operator const VkShaderModule& () const { return m_ShaderModule; }


private:
	// Member variables
	VkDevice m_Device{ nullptr };
	VkShaderModule m_ShaderModule{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif