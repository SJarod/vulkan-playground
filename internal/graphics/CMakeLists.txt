set(component graphics)

add_library(${component} STATIC "")

target_sources(${component}
    PRIVATE
    context.hpp
    context.cpp

    instance.hpp
    instance.cpp

    surface.hpp
    swapchain.hpp
)

target_link_libraries(${component}
    PUBLIC Vulkan-Headers
    PUBLIC FuUtilities
)

target_include_directories(${component} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")