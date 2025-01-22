set(component engine)

add_library(${component} INTERFACE "")

target_sources(${component}
    PRIVATE
    uniform.hpp
    vertex.hpp
)

target_link_libraries(${component}
    INTERFACE ${Vulkan_LIBRARY}
    INTERFACE glm
)

target_include_directories(${component} INTERFACE "${Vulkan_INCLUDE_DIR}")
target_include_directories(${component} INTERFACE "${glm_SOURCE_DIR}")

target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/..")