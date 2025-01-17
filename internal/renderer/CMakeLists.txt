set(component renderer)

add_library(${component} STATIC "")

target_sources(${component}
    PRIVATE
    renderer.hpp
    renderer.cpp
)

target_link_libraries(${component}
    PUBLIC graphics
    # PUBLIC ${Vulkan_LIBRARY}
)

# target_include_directories(${component} PUBLIC "${Vulkan_INCLUDE_DIR}")

target_include_directories(${component} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/..")