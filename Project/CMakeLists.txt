# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

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


set(SOURCES
    "main.cpp"
    "Utils.h"
    "Source/HelloTriangleApplication.h" "Source/HelloTriangleApplication.cpp"

    "Source/GP2_VkShaderModule.h" "Source/GP2_VkShaderModule.cpp"
    "Source/GP2_VkFence.h" "Source/GP2_VkFence.cpp"
    "Source/GP2_VkSemaphore.h" "Source/GP2_VkSemaphore.cpp"
    "Source/GP2_VkCommandPool.h" "Source/GP2_VkCommandPool.cpp"
    "Source/GP2_VkPipelineLayout.h" "Source/GP2_VkPipelineLayout.cpp"
    "Source/GP2_VkRenderPass.h" "Source/GP2_VkRenderPass.cpp"
    "Source/GP2_VkSwapchainKHR.h" "Source/GP2_VkSwapchainKHR.cpp"
    "Source/GP2_VkImageView.h" "Source/GP2_VkImageView.cpp"
    "Source/GP2_VkFramebuffer.h" "Source/GP2_VkFramebuffer.cpp"
    "Source/GP2_VkSurfaceKHR.h" "Source/GP2_VkSurfaceKHR.cpp"
    "Source/GP2_VkInstance.h" "Source/GP2_VkInstance.cpp"
    "Source/GP2_VkDevice.h" "Source/GP2_VkDevice.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES})
add_dependencies(${PROJECT_NAME} Shaders)
# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)