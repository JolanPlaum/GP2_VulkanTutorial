#ifndef GP2VKT_POOLCOMMANDBUFFERS_H_
#define GP2VKT_POOLCOMMANDBUFFERS_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include "RAII/GP2_VkCommandPool.h"

// Class Forward Declarations


// Class Declaration
class PoolCommandBuffers final
{
public:
	// Constructors and Destructor
	PoolCommandBuffers(const VkDevice& device, uint32_t queueFamilyIndex, uint32_t count);
	~PoolCommandBuffers() = default;
	
	// Copy and Move semantics
	PoolCommandBuffers(const PoolCommandBuffers& other)					= delete;
	PoolCommandBuffers& operator=(const PoolCommandBuffers& other)		= delete;
	PoolCommandBuffers(PoolCommandBuffers&& other) noexcept				= delete;
	PoolCommandBuffers& operator=(PoolCommandBuffers&& other) noexcept	= delete;

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