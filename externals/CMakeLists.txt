add_subdirectory(glfw)
add_subdirectory(volk)
add_subdirectory(Vulkan-Headers)
add_subdirectory(SPIRV-Headers)
add_subdirectory(SPIRV-Tools)
set(VUL_WERROR ON)
set(BUILD_TESTS OFF)
add_subdirectory(Vulkan-Utility-Libraries)
add_subdirectory(Vulkan-ValidationLayers)

# https://stackoverflow.com/questions/40294146/alternative-to-cmake-post-build-command-when-target-is-in-subdirectory
set(VkLayer_utils_filename VkLayer_khronos_validation.dll)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/${VkLayer_utils_filename}
    COMMAND echo "Set env var for VK_LAYER_PATH to : ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/"
    COMMAND VK_LAYER_PATH="${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/"
    DEPENDS VkLayer_utils
)
add_custom_target(VkLayer_utils_postbuild ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/${VkLayer_utils_filename}
)