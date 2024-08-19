#ifndef GP2VKT_GP2_SINGLETIMECOMMAND_H_
#define GP2VKT_GP2_SINGLETIMECOMMAND_H_
// Includes
#include <vulkan/vulkan_core.h>
#include "Source/PoolCommandBuffers.h"

// Class Forward Declarations


// RAII wrapper for SingleTimeCommands
class GP2_SingleTimeCommand final
{
public:
	// Constructors and Destructor
	GP2_SingleTimeCommand(VkDevice device, uint32_t queueFamilyIndex, VkQueue queue)
		: m_CommandBuffers{ device, queueFamilyIndex, 1 }
		, m_Queue{ queue }
	{
		// Specify command buffer usage
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Begin recording commands
		vkBeginCommandBuffer(m_CommandBuffers.Get()[0], &beginInfo);
	}
	~GP2_SingleTimeCommand()
	{
		// End recording commands
		vkEndCommandBuffer(m_CommandBuffers.Get()[0]);

		// Execute the commands
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers.Get()[0];

		vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_Queue);
	};
	
	// Copy and Move semantics
	GP2_SingleTimeCommand(const GP2_SingleTimeCommand& other)					= delete;
	GP2_SingleTimeCommand& operator=(const GP2_SingleTimeCommand& other)		= delete;
	GP2_SingleTimeCommand(GP2_SingleTimeCommand&& other) noexcept				= delete;
	GP2_SingleTimeCommand& operator=(GP2_SingleTimeCommand&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	VkCommandBuffer Get() { return m_CommandBuffers.Get()[0]; }


private:
	// Member variables
	PoolCommandBuffers m_CommandBuffers;
	VkQueue m_Queue{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif