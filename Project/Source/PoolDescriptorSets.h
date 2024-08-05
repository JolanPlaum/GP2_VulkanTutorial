#ifndef GP2VKT_POOLDESCRIPTORSETS_H_
#define GP2VKT_POOLDESCRIPTORSETS_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>
#include "RAII/GP2_VkDescriptorPool.h"

// Class Forward Declarations


// Class Declaration
class PoolDescriptorSets final
{
public:
	// Constructors and Destructor
	explicit PoolDescriptorSets(const VkDevice& device,
		const std::vector<VkDescriptorPoolSize>& poolSizes,
		const VkDescriptorSetLayout& layout,
		uint32_t count);
	~PoolDescriptorSets() = default;
	
	// Copy and Move semantics
	PoolDescriptorSets(const PoolDescriptorSets& other)					= delete;
	PoolDescriptorSets& operator=(const PoolDescriptorSets& other)		= delete;
	PoolDescriptorSets(PoolDescriptorSets&& other) noexcept				= delete;
	PoolDescriptorSets& operator=(PoolDescriptorSets&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	std::vector<VkDescriptorSet>& Get();


private:
	// Member variables
	GP2_VkDescriptorPool m_DescriptorPool{};
	std::vector<VkDescriptorSet> m_DescriptorSets{};

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif