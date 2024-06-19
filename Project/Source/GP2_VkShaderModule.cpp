//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "GP2_VkShaderModule.h"
#include "Utils.h"
#include <stdexcept>


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
GP2_VkShaderModule::GP2_VkShaderModule(const VkDevice& device, const std::string& path)
	: m_Device{ device }
	, m_ShaderModule{}
{
	// Load bytecode for the shader
	auto shaderCode = util::ReadFile(path);

	// Specify bytecode and size
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	// Create shader module using specified data
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_ShaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module!");
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
GP2_VkShaderModule::~GP2_VkShaderModule()
{
	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
VkShaderModule GP2_VkShaderModule::Get() const
{
	return m_ShaderModule;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

