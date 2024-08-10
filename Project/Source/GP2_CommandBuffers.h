#ifndef GP2VKT_GP2_COMMANDBUFFERS_H_
#define GP2VKT_GP2_COMMANDBUFFERS_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include "RAII/GP2_VkCommandPool.h"

// Class Forward Declarations


// Class Declaration
class GP2_CommandBuffers final
{
public:
	// Constructors and Destructor
	GP2_CommandBuffers(const VkDevice& device, uint32_t queueFamilyIndex, uint32_t count);
	~GP2_CommandBuffers() = default;
	
	// Copy and Move semantics
	GP2_CommandBuffers(const GP2_CommandBuffers& other)					= delete;
	GP2_CommandBuffers& operator=(const GP2_CommandBuffers& other)		= delete;
	GP2_CommandBuffers(GP2_CommandBuffers&& other) noexcept				= delete;
	GP2_CommandBuffers& operator=(GP2_CommandBuffers&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	std::vector<VkCommandBuffer>& Get();
	//TODO: add function to add/remove command buffers


private:
	// Member variables
	GP2_VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif