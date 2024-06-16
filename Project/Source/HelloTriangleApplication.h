#ifndef GP2VKT_HELLOTRIANGLEAPPLICATION_H_
#define GP2VKT_HELLOTRIANGLEAPPLICATION_H_
// Includes
#include <vulkan/vulkan_core.h>
#include <vector>

// Class Forward Declarations
class GLFWwindow;


// Class Declaration
class HelloTriangleApplication final
{
public:
	// Constructors and Destructor
	explicit HelloTriangleApplication() = default;
	~HelloTriangleApplication() = default;
	
	// Copy and Move semantics
	HelloTriangleApplication(const HelloTriangleApplication& other)					= delete;
	HelloTriangleApplication& operator=(const HelloTriangleApplication& other)		= delete;
	HelloTriangleApplication(HelloTriangleApplication&& other) noexcept				= delete;
	HelloTriangleApplication& operator=(HelloTriangleApplication&& other) noexcept	= delete;

	//---------------------------
	// Public Member Functions
	//---------------------------
	void Run();


private:
	// Member variables
	GLFWwindow* m_Window; // TODO: RAII
	VkInstance m_Instance; // TODO: RAII
	VkDebugUtilsMessengerEXT m_DebugMessenger; // TODO: RAII

	//---------------------------
	// Private Member Functions
	//---------------------------
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

	void CreateInstance();
	bool CheckValidationLayerSupport() const;
	std::vector<const char*> GetRequiredExtensions() const;

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
};
#endif