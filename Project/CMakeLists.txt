# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
set(RESOURCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
set(RESOURCE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Resources")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
 
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

add_custom_target(
    Resources
)
add_custom_command(
    TARGET Resources
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RESOURCE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_SOURCE_DIR} ${RESOURCE_BINARY_DIR}
)

file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/013ac3beddff3dbffafd5177e7972067cd2b5083/stb_image.h" "${SAVE_FILES_DIR}/stb_image.h")


set(SOURCES
    "main.cpp"
    "Utils.h"
    "DataTypes.h"
    "Source/HelloTriangleApplication.h" "Source/HelloTriangleApplication.cpp"

    "Source/PoolCommandBuffers.h" "Source/PoolCommandBuffers.cpp"
    "Source/PoolDescriptorSets.h" "Source/PoolDescriptorSets.cpp"

    "Source/RAII/GP2_VkShaderModule.h" "Source/RAII/GP2_VkShaderModule.cpp"
    "Source/RAII/GP2_VkFence.h" "Source/RAII/GP2_VkFence.cpp"
    "Source/RAII/GP2_VkSemaphore.h" "Source/RAII/GP2_VkSemaphore.cpp"
    "Source/RAII/GP2_VkCommandPool.h" "Source/RAII/GP2_VkCommandPool.cpp"
    "Source/RAII/GP2_VkPipelineLayout.h" "Source/RAII/GP2_VkPipelineLayout.cpp"
    "Source/RAII/GP2_VkRenderPass.h" "Source/RAII/GP2_VkRenderPass.cpp"
    "Source/RAII/GP2_VkSwapchainKHR.h" "Source/RAII/GP2_VkSwapchainKHR.cpp"
    "Source/RAII/GP2_VkImage.h" "Source/RAII/GP2_VkImage.cpp"
    "Source/RAII/GP2_VkImageView.h" "Source/RAII/GP2_VkImageView.cpp"
    "Source/RAII/GP2_VkFramebuffer.h" "Source/RAII/GP2_VkFramebuffer.cpp"
    "Source/RAII/GP2_VkSurfaceKHR.h" "Source/RAII/GP2_VkSurfaceKHR.cpp"
    "Source/RAII/GP2_VkInstance.h" "Source/RAII/GP2_VkInstance.cpp"
    "Source/RAII/GP2_VkDevice.h" "Source/RAII/GP2_VkDevice.cpp"
    "Source/RAII/GP2_VkBuffer.h" "Source/RAII/GP2_VkBuffer.cpp"
    "Source/RAII/GP2_VkDeviceMemory.h" "Source/RAII/GP2_VkDeviceMemory.cpp"
    "Source/RAII/GP2_VkDescriptorSetLayout.h" "Source/RAII/GP2_VkDescriptorSetLayout.cpp"
    "Source/RAII/GP2_VkDescriptorPool.h" "Source/RAII/GP2_VkDescriptorPool.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES})
add_dependencies(${PROJECT_NAME} Shaders Resources)
# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SAVE_FILES_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)