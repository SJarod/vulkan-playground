set(component graphics)

add_library(${component} STATIC "")

target_sources(${component}
    PRIVATE
    context.hpp
    context.cpp

    instance.hpp
    instance.cpp

    device.hpp
    device.cpp

    surface.hpp
    surface.cpp

    swapchain.hpp
    swapchain.cpp

    renderpass.hpp
    renderpass.cpp

    pipeline.hpp
    pipeline.cpp
)

target_link_libraries(${component}
    PUBLIC ${Vulkan_LIBRARY}
    PUBLIC engine
)

target_include_directories(${component} PUBLIC "${Vulkan_INCLUDE_DIR}")

target_include_directories(${component} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")