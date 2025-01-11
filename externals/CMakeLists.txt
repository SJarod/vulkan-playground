add_subdirectory(glfw)

add_subdirectory(volk)

add_subdirectory(Vulkan-Headers)
add_subdirectory(SPIRV-Headers)
add_subdirectory(SPIRV-Tools)
set(VUL_WERROR ON)
set(BUILD_TESTS OFF)
add_subdirectory(Vulkan-Utility-Libraries)

add_subdirectory(Vulkan-ValidationLayers)

# using layers built from source
# https://vulkan.lunarg.com/doc/view/1.3.296.0/windows/layer_configuration.html

# https://stackoverflow.com/a/40294859
set(VkLayer_utils_filename VkLayer_khronos_validation.dll)
# https://github.com/microsoft/vscode-cmake-tools/issues/1298
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/${VkLayer_utils_filename}
    COMMAND echo VkLayer_khronos_validation.dll built to ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/
    COMMAND echo set VK_LAYER_PATH environment variable to this path to use the validation layer
    DEPENDS VkLayer_utils
)
add_custom_target(VkLayer_utils_postbuild ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-ValidationLayers/layers/${CMAKE_BUILD_TYPE}/${VkLayer_utils_filename}
)