add_subdirectory(glfw)

find_package(Vulkan REQUIRED)
if (${Vulkan_FOUND})
    message("Found Vulkan version ${Vulkan_VERSION}")
endif()

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

add_subdirectory(glm)
